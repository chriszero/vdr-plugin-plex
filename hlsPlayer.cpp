#include "hlsPlayer.h"

#include <vdr/tools.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Exception.h>

#include <pcrecpp.h>
#include <algorithm>
#include <fstream>

#include "Plexservice.h"
#include "plexgdm.h"
#include "XmlObject.h"
#include "Media.h"
#include "Stream.h"

static cMutex hlsMutex;
const int BUFFERSIZE = 8192;

//--- cHlsSegmentLoader

cHlsSegmentLoader::cHlsSegmentLoader(std::string startm3u8, plexclient::cVideo *pVideo) {
    m_pVideo = pVideo;
    m_newList = false;
    m_bufferFilled = false;
    m_lastLoadedSegment = 0;
    m_segmentsToBuffer = 2;
    m_streamlenght = 0;
    m_lastSegmentSize = 0;
    m_pBuffer = new uchar[BUFFERSIZE];

    // Initialize members
    m_pClientSession = NULL;

    m_ringBufferSize = MEGABYTE(1);
    m_pRingbuffer = NULL;

    m_startUri = Poco::URI(startm3u8);
    m_startParser = cM3u8Parser();
    m_indexParser = cM3u8Parser();
}

cHlsSegmentLoader::~cHlsSegmentLoader() {
    // Stop Thread
    Cancel(0);

    delete m_pClientSession;
    delete[] m_pBuffer;
    delete m_pRingbuffer;
}

void cHlsSegmentLoader::SkipEmptySegments(int segmentDuration) {
    pcrecpp::RE re("&offset=(-?\\d+)", pcrecpp::RE_Options(PCRE_CASELESS));
    int value;
    re.PartialMatch(m_startUri.getQuery(), &value);
    if (value > segmentDuration) {
        m_lastLoadedSegment = (value / segmentDuration) - 1;
    }
}

bool cHlsSegmentLoader::LoadM3u8(std::string uri) {
    //LOCK_THREAD;
    m_startUri = Poco::URI(uri);
    return (m_newList = true);
}

void cHlsSegmentLoader::Action(void) {
    if (!LoadLists()) return;

    m_ringBufferSize = MEGABYTE(Config::GetInstance().BufferSize);

    isyslog("[plex]%s Create Ringbuffer %d MB", __FUNCTION__, Config::GetInstance().BufferSize);

    m_pRingbuffer = new cRingBufferLinear(m_ringBufferSize, 2 * TS_SIZE);

    while (Running()) {
        if (m_newList) {
            hlsMutex.Lock();
            m_bufferFilled = false;
            m_lastLoadedSegment = 0;
            LoadLists();
            m_newList = false;
            m_pRingbuffer->Clear();
            isyslog("[plex] Ringbuffer Cleared, loading new segments");
            hlsMutex.Unlock();
        }
        int count = 0;
        m_pRingbuffer->Get(count);
        if (!DoLoad() && count < TS_SIZE) {
            isyslog("[plex] No further segments to load and buffer empty, end of stream!");
            Cancel();

        }
        cCondWait::SleepMs(3);
    }
    StopLoader();
}

bool cHlsSegmentLoader::LoadLists(void) {
    if (LoadStartList()) {
        if (false == LoadIndexList()) {
            esyslog("[plex]LoadIndexList failed!");
            return false;
        }
    } else {
        esyslog("[plex]LoadStartList failed!");
        return false;
    }
    return true;
}

bool cHlsSegmentLoader::LoadIndexList(void) {
    bool res = false;
    try {
        if (m_startParser.MasterPlaylist && m_startParser.vPlaylistItems.size() > 0) {
            // Todo: make it universal, might only work for Plexmediaserver
            ConnectToServer();

            std::string startUri = m_startUri.toString();
            startUri = startUri.substr(0, startUri.find_last_of("/") + 1);

            Poco::URI indexUri(startUri + m_startParser.vPlaylistItems[0].file);

            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, indexUri.getPath());
            AddHeader(request);
            // same server
            m_pClientSession->sendRequest(request);

            Poco::Net::HTTPResponse responseStart;
            std::istream &indexFile = m_pClientSession->receiveResponse(responseStart);

            if (responseStart.getStatus() != 200) {
                esyslog("[plex]%s Response Not Valid", __FUNCTION__);
                Poco::StreamCopier::copyStream(indexFile, std::cout);
                return res;
            }
            m_indexParser = cM3u8Parser();
            res = m_indexParser.Parse(indexFile);

            if (res) {
                // Segment URI is relative to index.m3u8
                std::string path = indexUri.getPath();
                m_segmentUriPart = path.substr(0, path.find_last_of("/") + 1);
            }
            if (m_indexParser.TargetDuration > 3) {
                m_segmentsToBuffer = 2;
            } else {
                m_segmentsToBuffer = 3;
            }

            SkipEmptySegments(m_indexParser.TargetDuration);
            m_lastSegmentSize = 0;

            // Get stream lenght
            m_streamlenght = 0;
            for (unsigned int i = 0; i < m_indexParser.vPlaylistItems.size(); i++) {
                m_streamlenght += m_indexParser.vPlaylistItems[i].length;
            }
        }

    } catch (Poco::Exception &exc) {
        esyslog("[plex] %s %s", __FUNCTION__, exc.displayText().c_str());
        res = false;
    }
    return res;
}

bool cHlsSegmentLoader::LoadStartList(void) {
    bool res = false;
    ConnectToServer();
    try {
        dsyslog("[plex]%s %s", __FUNCTION__, m_startUri.toString().c_str());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, m_startUri.getPathAndQuery());
        AddHeader(request);
        m_pClientSession->sendRequest(request);

        Poco::Net::HTTPResponse responseStart;
        std::istream &startFile = m_pClientSession->receiveResponse(responseStart);

        if (responseStart.getStatus() != 200) {
            esyslog("[plex]%s Response Not Valid", __FUNCTION__);
            Poco::StreamCopier::copyStream(startFile, std::cout);
            return res;
        }

        m_startParser = cM3u8Parser();
        res = m_startParser.Parse(startFile);
        if (res) {
            // Get GUID
            pcrecpp::RE re("([0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12})",
                           pcrecpp::RE_Options(PCRE_CASELESS));
            string value;
            re.PartialMatch(m_startParser.vPlaylistItems[0].file, &value);
            m_sessionCookie = value;
        }
    } catch (Poco::Exception &exc) {
        esyslog("[plex] %s %s", __FUNCTION__, exc.displayText().c_str());
        res = false;
    }
    return res;
}

int cHlsSegmentLoader::EstimateSegmentSize() {
    if (&m_startParser.vPlaylistItems[0] == NULL) {
        esyslog("[plex]%s first element NULL", __FUNCTION__);
    }
    double bandw = m_startParser.vPlaylistItems[0].bandwidth / 8.0 / 1000.0 / 1000.0;

    int len = m_indexParser.TargetDuration;
    double estSize = (bandw) * len;
    estSize = std::max(estSize, 1.0);
    // default
    if (estSize <= 1) {
        estSize = 32;
    }
    return ceil(estSize);
}

bool cHlsSegmentLoader::LoadSegment(std::string segmentUri) {
    try {
        Poco::Net::HTTPRequest segmentRequest(Poco::Net::HTTPRequest::HTTP_GET, segmentUri);
        AddHeader(segmentRequest);
        m_pClientSession->sendRequest(segmentRequest);
    } catch (Poco::Exception &) {
        esyslog("[plex] %s; %s failed.", __FUNCTION__, segmentUri.c_str());
        return false;
    }
    Poco::Net::HTTPResponse segmentResponse;
    std::istream &segmentFile = m_pClientSession->receiveResponse(segmentResponse);

    if (segmentResponse.getStatus() != 200) {
        // error
        esyslog("[plex] %s; %s failed.", __FUNCTION__, segmentUri.c_str());
        return false;
    }

    if (segmentResponse.getContentLength() <= TS_SIZE) {
        // Empty segment
        dsyslog("[plex] %s: %s <= 188 byte.", __FUNCTION__, segmentUri.c_str());
        return true;
    }

    dsyslog("[plex] %s: %s successfully.", __FUNCTION__, segmentUri.c_str());
    // copy response
    segmentFile.read(reinterpret_cast<char *>(m_pBuffer), BUFFERSIZE);
    std::streamsize n = segmentFile.gcount();
    while (n > 0) {
        if (m_pRingbuffer->Free() >= n) {
            m_pRingbuffer->Put(m_pBuffer, n);

            segmentFile.read(reinterpret_cast<char *>(m_pBuffer), BUFFERSIZE);
            n = segmentFile.gcount();
            m_bufferFilled = true;
        }
        else {
            cCondWait::SleepMs(1);
        }
        if (m_newList) {
            break;
        }
    }
    return true;
}

int cHlsSegmentLoader::GetSegmentSize(int segmentIndex) {
    //dsyslog("[plex]%s Segment %d", __FUNCTION__, segmentIndex);
    if (m_indexParser.vPlaylistItems[segmentIndex].size >= TS_SIZE) {
        return m_indexParser.vPlaylistItems[segmentIndex].size;
    }
    try {
        Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_HEAD, GetSegmentUri(segmentIndex));
        AddHeader(req);
        m_pClientSession->sendRequest(req);
        Poco::Net::HTTPResponse reqResponse;
        m_pClientSession->receiveResponse(reqResponse);
        return m_indexParser.vPlaylistItems[segmentIndex].size = reqResponse.getContentLength();
    } catch (Poco::IOException &exc) {
        esyslog("[plex]%s %s ", __FUNCTION__, exc.displayText().c_str());
        return 0;
    }
}

std::string cHlsSegmentLoader::GetSegmentUri(int segmentIndex) const {
    return m_segmentUriPart + m_indexParser.vPlaylistItems[segmentIndex].file;
}

void cHlsSegmentLoader::CloseConnection(void) {
    if (m_pClientSession)
        m_pClientSession->abort();

    delete m_pClientSession;
    m_pClientSession = NULL;
}

bool cHlsSegmentLoader::ConnectToServer(void) {
    dsyslog("[plex]%s", __FUNCTION__);
    if (!m_pClientSession) {
        if (m_startUri.getScheme().find("https") != std::string::npos) {
            m_pClientSession = new Poco::Net::HTTPSClientSession(m_startUri.getHost(), m_startUri.getPort());
        }
        else {
            m_pClientSession = new Poco::Net::HTTPClientSession(m_startUri.getHost(), m_startUri.getPort());
        }
    }

    return m_pClientSession->connected();
}

bool cHlsSegmentLoader::DoLoad(void) {
    LOCK_THREAD;
    bool result = true;
    bool recover = false;
    if (m_lastLoadedSegment < m_indexParser.vPlaylistItems.size()) {

        std::string segmentUri = GetSegmentUri(m_lastLoadedSegment);
        if ((result = LoadSegment(segmentUri))) {
            m_lastLoadedSegment++;
        } else {
            // transcoder may be died, plex bug, restart transcode session
            esyslog("[plex] %s 404, Transcoder died, see logfile from PMS", __FUNCTION__);
            recover = true;
            std::string stopUri = "/video/:/transcode/universal/stop?session=" + m_sessionCookie;
            try {

                bool ok;
                auto cSession = m_pVideo->m_pServer->MakeRequest(ok, stopUri);
                int tmp = m_lastLoadedSegment;
                int tmp2 = m_lastSegmentSize;
                CloseConnection();
                LoadLists();
                m_lastLoadedSegment = tmp;
                m_lastSegmentSize = tmp2;
            } catch (Poco::Exception &) {
                return false;
            }
        }

        m_bufferFilled = result;
    } else {
        result = false;
    }
    return recover || result;
}

bool cHlsSegmentLoader::BufferFilled(void) {
    return m_bufferFilled;
}

bool cHlsSegmentLoader::StopLoader(void) {
    dsyslog("[plex]%s", __FUNCTION__);
    try {
        std::string stopUri = "/video/:/transcode/universal/stop?session=" + m_sessionCookie;

        bool ok;
        auto cSession = m_pVideo->m_pServer->MakeRequest(ok, stopUri);

        Cancel();

        return ok;
    } catch (Poco::Exception &exc) {
        esyslog("[plex]%s %s ", __FUNCTION__, exc.displayText().c_str());
        return false;
    }
}

void cHlsSegmentLoader::AddHeader(Poco::Net::HTTPRequest &req) {
    req.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
//	req.add("X-Plex-Device", "PC");
    req.add("X-Plex-Model", "Linux");
    if (Config::GetInstance().UseCustomTranscodeProfile) {
        req.add("X-Plex-Device", "VDR Plex Plugin");
    } else {
        req.add("X-Plex-Device", "Plex Home Theater");
        //req.add("X-Plex-Device", "tvOS");
    }

    if (Config::GetInstance().UsePlexAccount && !m_pVideo->m_pServer->GetAuthToken().empty()) {
        // Add PlexToken to Header
        req.add("X-Plex-Token", m_pVideo->m_pServer->GetAuthToken());
    }
}

bool cHlsSegmentLoader::Active(void) {
    return Running();
}

void cHlsSegmentLoader::Ping(void) {
    dsyslog("[plex]%s", __FUNCTION__);
    try {
        std::string uri = "/video/:/transcode/universal/ping?session=" + Config::GetInstance().GetUUID();

        bool ok;
        auto cSession = m_pVideo->m_pServer->MakeRequest(ok, uri);

    } catch (Poco::Exception &exc) {
        esyslog("[plex]%s %s ", __FUNCTION__, exc.displayText().c_str());
    }
}

void cHlsSegmentLoader::ResizeRingbuffer(int newsize) {
    hlsMutex.Lock();
    isyslog("[plex] %s, Oldsize: %d, Newsize: %d", __FUNCTION__, m_ringBufferSize, newsize);
    //Create new Ringbuffer
    cRingBufferLinear *newBuffer = new cRingBufferLinear(newsize, TS_SIZE);
    // Copy old data
    int count = 0;
    uchar *pData = m_pRingbuffer->Get(count);
    newBuffer->Put(pData, count);
    // delete old buffer
    delete m_pRingbuffer;
    m_pRingbuffer = NULL;
    // assing new buffer
    m_pRingbuffer = newBuffer;
    m_ringBufferSize = newsize;
    hlsMutex.Unlock();
}

int cHlsSegmentLoader::GetStreamLenght() {
    return m_streamlenght;
}

//--- cHlsPlayer

cHlsPlayer::cHlsPlayer(std::string startm3u8, plexclient::cVideo Video, int offset) {
    dsyslog("[plex]: '%s'", __FUNCTION__);
    m_Video = Video;
    m_timeOffset = offset;
    m_jumpOffset = 0;
    m_tTimeSum = 0;
    m_doJump = false;
    m_isBuffering = false;
    AudioIndexOffset = 1000; // Just a magic number
    m_tTimer.Set(1);
    m_pSegmentLoader = new cHlsSegmentLoader(startm3u8, &m_Video);

    m_pDebugFile = NULL;
    //m_pDebugFile = new std::ofstream();
    //m_pDebugFile->open("debug.ts", std::ios::out);
}

cHlsPlayer::~cHlsPlayer() {
    dsyslog("[plex]: '%s'", __FUNCTION__);
    Cancel();
    if (m_pDebugFile) m_pDebugFile->close();
    delete m_pDebugFile;
    m_pDebugFile = NULL;

    delete m_pSegmentLoader;
    m_pSegmentLoader = NULL;
    Detach();
}

void cHlsPlayer::SetAudioAndSubtitleTracks(void) {
    //DeviceClrAvailableTracks();
    DeviceSetAvailableTrack(ttDolby, 0, 0, "Current");
    if (m_Video.m_Media.m_vStreams.size() > 0) {
        std::vector<plexclient::Stream> streams = m_Video.m_Media.m_vStreams;
        for (std::vector<plexclient::Stream>::iterator it = streams.begin(); it != streams.end(); ++it) {
            plexclient::Stream *pStream = &(*it);
            if (pStream->m_eStreamType == plexclient::StreamType::sAUDIO) {
                DeviceSetAvailableTrack(ttDolby, pStream->m_iIndex, pStream->m_iIndex + AudioIndexOffset,
                                        pStream->m_sLanguage.c_str(), pStream->m_sCodec.c_str());
            }
        }
    }
}


void cHlsPlayer::Action(void) {
    // Start SegmentLoader
    m_pSegmentLoader->Start();

    m_bFirstPlay = true;

    while (Running()) {
        if (m_doJump && m_pSegmentLoader && m_pSegmentLoader->BufferFilled()) {
            LOCK_THREAD;

            DeviceFreeze();
            m_doJump = false;
            m_bFirstPlay = true;
            std::string uri = plexclient::Plexservice::GetUniversalTranscodeUrl(&m_Video, m_jumpOffset);
            m_pSegmentLoader->LoadM3u8(uri);
            m_timeOffset = m_jumpOffset;
            DeviceClear();
            DevicePlay();
            playMode = pmPlay;
        } else if (m_pSegmentLoader && m_pSegmentLoader->BufferFilled()) {
            DoPlay();
            if (m_bFirstPlay) {
                SetAudioAndSubtitleTracks();
                ResetPlayedSeconds();
                playMode = pmPlay;
                m_bFirstPlay = false;
            }
            CountPlayedSeconds();
        } else {
            // Pause
            cCondWait::SleepMs(3);
        }
        // statusupdates to pms every 60s
        if (m_pSegmentLoader && m_tTimer.TimedOut()) {
            if (playMode == pmPause) {
                m_pSegmentLoader->Ping();
            }
            ReportProgress();
            m_tTimer.Set(60000);
        }
    }
    // set as watched if >= 90%
    int t = m_Video.m_Media.m_lDuration / 1000 * 0.9;
    if (GetPlayedSeconds() >= t) {
        SetWatched();
    }
    DeviceClear();
}

bool cHlsPlayer::DoPlay(void) {
    bool res = false;
    cPoller Poller;
    if (DevicePoll(Poller, 10)) {
        hlsMutex.Lock();

// Handle running out of packets. Buffer-> Play-> Pause-> Buffer-> Play

        for (int i = 0; i < 10; i++) {
            // Get a pointer to start of the data and the number of avaliable bytes
            int bytesAvaliable = 0;
            uchar *toPlay = m_pSegmentLoader->m_pRingbuffer->Get(bytesAvaliable);
            if (bytesAvaliable >= TS_SIZE) {
                int playedBytes = PlayTs(toPlay, TS_SIZE, false);
                // save stream to disk to debug data
                if (m_pDebugFile) {
                    m_pDebugFile->write((char *) toPlay, playedBytes);
                }

                m_pSegmentLoader->m_pRingbuffer->Del(playedBytes);
                res = true;
            } else {
                // Pause & Buffer
                break;
            }
        }
        hlsMutex.Unlock();
    }
    return res;
}

void cHlsPlayer::Activate(bool On) {
    if (On) {
        Start();
    } else {
        Cancel(1);
    }
}

bool cHlsPlayer::GetIndex(int &Current, int &Total, bool SnapToIFrame __attribute__((unused))) {
    if (m_Video.m_Media.m_lDuration > 0) {
        Total = m_Video.m_Media.m_lDuration / 1000 * FramesPerSecond(); // milliseconds
    } else {
        Total = m_pSegmentLoader->GetStreamLenght() * FramesPerSecond();
    }
    Current = GetPlayedSeconds() * FramesPerSecond();
    return true;
}

bool cHlsPlayer::GetReplayMode(bool &Play, bool &Forward, int &Speed) {
    Play = (playMode == pmPlay);
    Forward = true;
    Speed = -1;
    return true;
}

void cHlsPlayer::Pause(void) {
    LOCK_THREAD;
    dsyslog("[plex]%s", __FUNCTION__);
    if (playMode == pmPause) {
        Play();
    } else {
        DeviceFreeze();
        playMode = pmPause;
    }
}

void cHlsPlayer::Play(void) {
    LOCK_THREAD;
    dsyslog("[plex]%s", __FUNCTION__);
    if (playMode != pmPlay) {
        DevicePlay();
        playMode = pmPlay;
    }
}

bool cHlsPlayer::Active(void) {
    return Running() && m_pSegmentLoader && m_pSegmentLoader->Active();
}

void cHlsPlayer::Stop(void) {
    LOCK_THREAD;
    dsyslog("[plex]%s", __FUNCTION__);
    ReportProgress(true);
    if (m_pSegmentLoader)
        m_pSegmentLoader->StopLoader();
    Cancel(1);
}

double cHlsPlayer::FramesPerSecond(void) {
    return m_Video.m_Media.m_VideoFrameRate == "24p" ? 24 : DEFAULTFRAMESPERSECOND;
}

void cHlsPlayer::JumpRelative(int seconds) {
    JumpTo(GetPlayedSeconds() + seconds);
}

void cHlsPlayer::JumpTo(int seconds) {
    LOCK_THREAD;
    dsyslog("[plex]%s %d", __FUNCTION__, seconds);
    if (seconds < 0) seconds = 0;
    m_jumpOffset = seconds;
    m_doJump = true;
}

void cHlsPlayer::SetAudioTrack(eTrackType Type __attribute__((unused)), const tTrackId *TrackId) {
    LOCK_THREAD;
    dsyslog("[plex]%s %d %s", __FUNCTION__, TrackId->id, TrackId->language);
    // Check if stream availiable
    int streamId = 0;
    if (m_Video.m_Media.m_vStreams.size() > 0) {
        std::vector<plexclient::Stream> streams = m_Video.m_Media.m_vStreams;
        for (std::vector<plexclient::Stream>::iterator it = streams.begin(); it != streams.end(); ++it) {
            plexclient::Stream *pStream = &(*it);
            if (pStream->m_eStreamType == plexclient::StreamType::sAUDIO &&
                pStream->m_iIndex + AudioIndexOffset == TrackId->id) {
                streamId = pStream->m_iID;
                break;
            }
        }
    }
    // Then do the request
    if (streamId > 0) {
        std::string uri = "/library/parts/" + std::string(itoa(m_Video.m_Media.m_iPartId)) + "?audioStreamID=" +
                          std::string(itoa(streamId));

        bool ok;
        auto cSession = m_Video.m_pServer->MakeRequest(ok, uri);
        if (ok) {
            DeviceSetCurrentAudioTrack(eTrackType(ttDolby + 0)); // hacky
            DeviceSetAvailableTrack(ttDolby, 0, 0, TrackId->language);
            JumpRelative(0); // Reload Stream to get new Audio
            dsyslog("[plex]: Set AudioStream: %d\n", TrackId->id);
        }
    }
}

int cHlsPlayer::GetPlayedSeconds(void) {
    return m_timeOffset + (m_tTimeSum / 1000);
}

void cHlsPlayer::CountPlayedSeconds(void) {
    if (playMode == pmPlay) {
        unsigned long long tTmp = cTimeMs::Now();
        m_tTimeSum += (tTmp - m_tLastTime);
        m_tLastTime = tTmp;
    } else {
        m_tLastTime = cTimeMs::Now();
    }
}

void cHlsPlayer::ResetPlayedSeconds(void) {
    m_tTimeSum = 0;
    m_tLastTime = cTimeMs::Now();
}

void cHlsPlayer::ReportProgress(bool stopped) {
    std::string state;
    if (stopped) {
        state = "stopped";
    } else if (playMode == pmPlay) {
        state = "playing";
    } else {
        state = "paused";
    }

    try {
        std::string uri = "/:/progress?key=" + std::string(itoa(m_Video.m_iRatingKey)) +
                          "&identifier=com.plexapp.plugins.library&time=" +
                          std::string(itoa(GetPlayedSeconds() * 1000)) + "&state=" + state;

        bool ok;
        auto cSession = m_Video.m_pServer->MakeRequest(ok, uri);

        if (ok) {
            dsyslog("[plex] %s", __FUNCTION__);
        }
    } catch (Poco::Exception &) { }

}

void cHlsPlayer::SetWatched(void) {
    std::string uri =
            "/:/scrobble?key=" + std::string(itoa(m_Video.m_iRatingKey)) + "&identifier=com.plexapp.plugins.library";

    bool ok;
    auto cSession = m_Video.m_pServer->MakeRequest(ok, uri);

    if (ok) {
        dsyslog("[plex] %s", __FUNCTION__);
    }
}
