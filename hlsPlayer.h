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
#include "PVideo.h"
#include "Media.h"

class cHlsSegmentLoader : public cThread {
private:
    int m_ringBufferSize;
    unsigned int m_segmentsToBuffer;
    unsigned int m_lastLoadedSegment;
    int m_lastSegmentSize;
    bool m_bufferFilled;
    bool m_newList;
    int m_streamlenght;

    uchar *m_pBuffer;

    Poco::Net::HTTPClientSession *m_pClientSession;
    plexclient::cVideo *m_pVideo;
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

    bool LoadLists(void);

    void ResizeRingbuffer(int newsize);

    void SkipEmptySegments(int segmentLenght);


protected:
    void Action(void);

    bool DoLoad(void);

public:
    cHlsSegmentLoader(std::string startm3u8, plexclient::cVideo *pVideo);

    ~cHlsSegmentLoader();

    cRingBufferLinear *m_pRingbuffer;

    bool BufferFilled(void);

    bool Active(void);

    bool StopLoader(void);

    bool LoadM3u8(std::string uri);

    void AddHeader(Poco::Net::HTTPRequest &req);

    void Ping(void);

    int GetStreamLenght();
};

class cHlsPlayer : public cPlayer, cThread {
private:
    std::ofstream *m_pDebugFile;
    int AudioIndexOffset;
    cHlsSegmentLoader *m_pSegmentLoader;
    plexclient::cVideo m_Video;

    int m_jumpOffset;
    int m_timeOffset;
    bool m_doJump;
    bool m_isBuffering;

    int m_videoLenght;
    int m_actualSegment;
    int m_actualTime;
    long m_lastValidSTC;

    unsigned long long m_tLastTime;
    unsigned long long m_tTimeSum;
    bool m_bFirstPlay;
    cTimeMs m_tTimer;

    enum ePlayModes {
        pmPlay, pmPause
    };
    ePlayModes playMode;

    virtual void Activate(bool On);

    int GetPlayedSeconds(void);

    void CountPlayedSeconds(void);

    void ResetPlayedSeconds(void);

    void ReportProgress(bool stopped = false);

    void SetWatched(void);

protected:
    void Action(void);

    bool DoPlay(void);


public:
    //static cMutex s_mutex;
    cHlsPlayer(std::string startm3u8, plexclient::cVideo Video, int offset = 0);

    ~cHlsPlayer();

    virtual bool GetIndex(int &Current, int &Total, bool SnapToIFrame = false);

    virtual bool GetReplayMode(bool &Play, bool &Forward, int &Speed);

    virtual double FramesPerSecond(void);

    virtual void SetAudioTrack(eTrackType Type, const tTrackId *TrackId);

    void Pause(void);

    void Play(void);

    void Stop(void);

    bool Active(void);

    void JumpTo(int seconds);

    void JumpRelative(int seconds);

    void SetAudioAndSubtitleTracks(void);

};

#endif // HLSPLAYER_H
