#include "hlsPlayer.h"

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>

#include <pcrecpp.h>

//--- cHlsSegmentLoader

cHlsSegmentLoader::cHlsSegmentLoader(std::string startm3u8)
{
	m_bufferFilled = false;
	m_lastLoadedSegment = 0;
	m_segmentsToBuffer = 3;
	m_pBuffer = new uchar[8192];

	// Initialize members
	m_pClientSession = NULL;

	m_ringBufferSize = MEGABYTE(32);
	m_pRingbuffer = NULL;

	m_startUri = Poco::URI(startm3u8);
	m_startParser = cM3u8Parser();
	m_indexParser = cM3u8Parser();
}

cHlsSegmentLoader::~cHlsSegmentLoader()
{
	// Stop Thread
	Cancel(0);

	delete m_pClientSession;
	delete[] m_pBuffer;
	delete m_pRingbuffer;
}

void cHlsSegmentLoader::Action(void)
{
	LoadStartList();
	LoadIndexList();

	int estSize = EstimateSegmentSize();
	m_ringBufferSize = MEGABYTE(estSize*3);

	std::cout << "Create Ringbuffer " << estSize*3 << "MB" << std::endl;

	m_pRingbuffer = new cRingBufferLinear(m_ringBufferSize, 2*TS_SIZE);

	while(Running()) {
		DoLoad();
		cCondWait::SleepMs(3);
	}
}

void cHlsSegmentLoader::LoadIndexList(void)
{
	if(m_startParser.MasterPlaylist && m_startParser.vPlaylistItems.size() > 0) {
		// Todo: make it universal, might only work for Plexmediaserver
		ConnectToServer();

		std::string startUri = m_startUri.toString();
		startUri = startUri.substr(0, startUri.find_last_of("/")+1);

		Poco::URI indexUri(startUri+m_startParser.vPlaylistItems[0].file);

		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, indexUri.getPath());
		// same server
		m_pClientSession->sendRequest(request);

		Poco::Net::HTTPResponse responseStart;
		std::istream& indexFile = m_pClientSession->receiveResponse(responseStart);

		if(responseStart.getStatus() != 200) {
			// error
			return;
		}

		m_indexParser.Parse(indexFile);

		// Segment URI is relative to index.m3u8
		std::string path = indexUri.getPath();
		m_segmentUriPart = path.substr(0, path.find_last_of("/")+1);
	}
}

void cHlsSegmentLoader::LoadStartList(void)
{
	ConnectToServer();

	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, m_startUri.getPathAndQuery());
	m_pClientSession->sendRequest(request);

	Poco::Net::HTTPResponse responseStart;
	std::istream& startFile = m_pClientSession->receiveResponse(responseStart);

	if(responseStart.getStatus() != 200) {
		// error
		return;
	}

	m_startParser.Parse(startFile);

	pcrecpp::RE re("([0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12})", pcrecpp::RE_Options(PCRE_CASELESS));
	string value;
	re.PartialMatch(m_startParser.vPlaylistItems[0].file, &value);
	m_sessionCookie = value;
}

int cHlsSegmentLoader::EstimateSegmentSize()
{
	double bandw = m_startParser.vPlaylistItems[0].bandwidth / 8.0 / 1000.0 / 1000.0;

	int len = m_indexParser.TargetDuration;
	double estSize = (bandw) * len;
	estSize = max(estSize, 16.0); // default
	return ceil(estSize);
}

bool cHlsSegmentLoader::LoadSegment(std::string segmentUri)
{
	std::cout << "Loading Segment: " << segmentUri << "... ";
	Poco::Net::HTTPRequest segmentRequest(Poco::Net::HTTPRequest::HTTP_GET, segmentUri);
	m_pClientSession->sendRequest(segmentRequest);

	Poco::Net::HTTPResponse segmentResponse;
	std::istream& segmentFile = m_pClientSession->receiveResponse(segmentResponse);

	if(segmentResponse.getStatus() != 200) {
		// error
		std::cout << "failed." << std::endl;
		return false;
	}
	std::cout << "successfully." << std::endl;

	// copy response

	int m = 0;
	segmentFile.read(reinterpret_cast<char*>(m_pBuffer), sizeof(m_pBuffer));
	std::streamsize n = segmentFile.gcount();
	while(n > 0) {
		m = m_pRingbuffer->Put(m_pBuffer, n);
		if(m < n) {
			// oops, this should not happen. Data doesn't fitted completly into ringbuffer
			break;
		} else {
			segmentFile.read(reinterpret_cast<char*>(m_pBuffer), sizeof(m_pBuffer));
			n = segmentFile.gcount();
		}
	}
	return true;
}

int cHlsSegmentLoader::GetSegmentSize(int segmentIndex)
{
	if(m_indexParser.vPlaylistItems[segmentIndex].size > 0) {
		return m_indexParser.vPlaylistItems[segmentIndex].size;
	}
	Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_HEAD, GetSegmentUri(segmentIndex));
	m_pClientSession->sendRequest(req);
	Poco::Net::HTTPResponse reqResponse;
	m_pClientSession->receiveResponse(reqResponse);

	return m_indexParser.vPlaylistItems[segmentIndex].size = reqResponse.getContentLength();
}

std::string cHlsSegmentLoader::GetSegmentUri(int segmentIndex) const
{
	return m_segmentUriPart + m_indexParser.vPlaylistItems[segmentIndex].file;
}

void cHlsSegmentLoader::CloseConnection(void)
{
	if(m_pClientSession)
		m_pClientSession->abort();

	delete m_pClientSession;
	m_pClientSession = NULL;
}

bool cHlsSegmentLoader::ConnectToServer(void)
{
	if(!m_pClientSession)
		m_pClientSession = new Poco::Net::HTTPClientSession(m_startUri.getHost(), m_startUri.getPort());

	return m_pClientSession->connected();
}

bool cHlsSegmentLoader::DoLoad(void)
{
	bool result = true;

	int nextSegmentSize = GetSegmentSize(m_lastLoadedSegment + 1);
	while(m_pRingbuffer->Free() > nextSegmentSize) {

		if(m_lastLoadedSegment + 1 <  m_indexParser.vPlaylistItems.size()) {
			std::string segmentUri = GetSegmentUri(m_lastLoadedSegment + 1);
			result = LoadSegment(segmentUri);
			m_lastLoadedSegment++;
		} else {
			// out of segments
		}
		nextSegmentSize = GetSegmentSize(m_lastLoadedSegment + 1);
	}
	return m_bufferFilled = result;
}

bool cHlsSegmentLoader::BufferFilled(void)
{
	return m_bufferFilled;
}

bool cHlsSegmentLoader::StopLoader(void)
{
	std::string stopUri = "/video/:/transcode/segmented/stop?session=" + m_sessionCookie;
	Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, stopUri);
	m_pClientSession->sendRequest(req);
	Poco::Net::HTTPResponse reqResponse;
	m_pClientSession->receiveResponse(reqResponse);
	
	Cancel();
	
	return reqResponse.getStatus() == 200;
}

//--- cHlsPlayer

cHlsPlayer::cHlsPlayer(std::string startm3u8)
{
	m_pSegmentLoader = new cHlsSegmentLoader(startm3u8);
}

cHlsPlayer::~cHlsPlayer()
{
	delete m_pSegmentLoader;
}


void cHlsPlayer::Action(void)
{
	// Start SegmentLoader
	m_pSegmentLoader->Start();

	while (Running()) {
		if(m_pSegmentLoader->BufferFilled()) {
			DoPlay();
		} else {
			// Pause
			cCondWait::SleepMs(3);
		}
	}
}

bool cHlsPlayer::DoPlay(void)
{
	cPoller Poller;
	if(DevicePoll(Poller, 10)) {
		LOCK_THREAD;

// Handle running out of packets. Buffer-> Play-> Pause-> Buffer-> Play

		for(int i = 0; i < 100; i++) {
			// Get a pointer to start of the data and the number of avaliable bytes
			int bytesAvaliable = 0;
			uchar* toPlay = m_pSegmentLoader->m_pRingbuffer->Get(bytesAvaliable);
			if(bytesAvaliable >= TS_SIZE) {
				int playedBytes = PlayTs(toPlay, TS_SIZE, false);
				m_pSegmentLoader->m_pRingbuffer->Del(playedBytes);
			} else {
				// Pause & Buffer
				break;
			}
		}
	}
	return true;
}

void cHlsPlayer::Activate(bool On)
{
	if(On) {
		Start();
	} else {
		Cancel(1);
	}
}

bool cHlsPlayer::GetIndex(int& Current, int& Total, bool SnapToIFrame)
{
	Total = 9999;
	Current = -1;
	return true;

}

bool cHlsPlayer::GetReplayMode(bool& Play, bool& Forward, int& Speed)
{
	Play = (playMode == pmPlay);
	Forward = true;
	Speed = -1;
	return true;
}

void cHlsPlayer::Pause(void)
{
	// from vdr-1.7.34
	if (playMode == pmPause) {
		Play();
	} else {
		LOCK_THREAD;

		DeviceFreeze();
		playMode = pmPause;
	}
}

void cHlsPlayer::Play(void)
{
	// from vdr-1.7.34
	if (playMode != pmPlay) {
		LOCK_THREAD;

		DevicePlay();
		playMode = pmPlay;
	}
}
