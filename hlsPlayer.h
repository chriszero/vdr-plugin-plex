#ifndef HLSPLAYER_H
#define HLSPLAYER_H

#include <string>
#include <iostream>
#include <sstream>

#include <vdr/thread.h>
#include <vdr/player.h>
#include <vdr/ringbuffer.h>
#include <vdr/tools.h>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include "m3u8Parser.h"
#include "Config.h"

class cHlsSegmentLoader : public cThread
{
private:
	int m_ringBufferSize;
	int m_segmentsToBuffer;
	unsigned int m_lastLoadedSegment;
	bool m_bufferFilled;

	uchar* m_pBuffer;

	Poco::Net::HTTPClientSession* m_pClientSession;
	Poco::URI m_startUri;
	std::string m_sessionUriPart;
	std::string m_segmentUriPart;
	std::string m_sessionCookie;

	cM3u8Parser m_startParser;
	cM3u8Parser m_indexParser;

	bool ConnectToServer(void);
	void CloseConnection(void);
	bool LoadStartList(void);
	bool LoadIndexList(void);
	std::string GetSegmentUri(int segmentIndex) const;
	int GetSegmentSize(int segmentIndex);
	bool LoadSegment(std::string uri);
	int EstimateSegmentSize();

protected:
	void Action(void);
	bool DoLoad(void);
	void AddHeader(Poco::Net::HTTPRequest& req);
	bool StopLoader(void);

public:
	cHlsSegmentLoader(std::string startm3u8);
	~cHlsSegmentLoader();

	cRingBufferLinear*  m_pRingbuffer;
	bool BufferFilled();
};

class cHlsPlayer : public cPlayer, cThread
{
private:
	cHlsSegmentLoader* m_pSegmentLoader;

	int m_videoLenght;
	int m_actualSegment;
	int m_actualTime;

	enum ePlayModes { pmPlay, pmPause };
	ePlayModes playMode;

	virtual void Activate(bool On);


protected:
	void Action(void);
	bool DoPlay(void);


public:
	cHlsPlayer(std::string startm3u8);
	~cHlsPlayer();

	virtual bool GetIndex(int &Current, int &Total, bool SnapToIFrame = false);
	virtual bool GetReplayMode(bool &Play, bool &Forward, int &Speed);
	void Pause(void);
	void Play(void);

};

#endif // HLSPLAYER_H
