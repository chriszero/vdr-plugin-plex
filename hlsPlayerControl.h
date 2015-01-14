#ifndef HLSPLAYERCONTROL_H
#define HLSPLAYERCONTROL_H

#include <vdr/player.h>
#include <vdr/tools.h>

#include "hlsPlayer.h"

class cHlsPlayerControl : public cControl
{
private:
	cHlsPlayer* player;
	std::string m_title;
	
	bool visible;

protected:
	//void ShowMode();

public:
	cHlsPlayerControl(cHlsPlayer* Player, std::string title);
	virtual ~cHlsPlayerControl();

	virtual void Show(void);
	virtual void Hide(void);

	virtual cString GetHeader(void);
	virtual eOSState ProcessKey(eKeys Key);

	bool Active(void);

	void Pause(void);
	void Play(void);
	void Stop(void);


};

#endif // HLSPLAYERCONTROL_H
