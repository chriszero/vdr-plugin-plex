#ifndef HLSPLAYERCONTROL_H
#define HLSPLAYERCONTROL_H

#include <vdr/player.h>
#include <vdr/tools.h>

#include "hlsPlayer.h"
#include "MediaContainer.h"
#include "PVideo.h"

class cHlsPlayerControl : public cControl
{
private:
	static volatile int active;
	cHlsPlayer* player;
	std::string m_title;

	cSkinDisplayReplay *displayReplay;
	bool visible, modeOnly, shown;
	int lastCurrent, lastTotal;
	bool lastPlay, lastForward;
	int lastSpeed;
	time_t timeoutShow;

	void ShowMode(void);
	bool ShowProgress(bool Initial);
	void ShowTimed(int Seconds = 0);

protected:
	//void ShowMode();

public:
	plexclient::Video m_Video;

	static cControl* Create(plexclient::Video Video);
	cHlsPlayerControl(cHlsPlayer* Player, plexclient::Video Video);
	virtual ~cHlsPlayerControl();

	virtual void Show(void);
	virtual void Hide(void);

	virtual cString GetHeader(void);
	virtual eOSState ProcessKey(eKeys Key);

	bool Active(void);

	void Pause(void);
	void Play(void);
	void Stop(void);
	void SeekTo(int offset);
	void JumpRelative(int offset);


};

#endif // HLSPLAYERCONTROL_H
