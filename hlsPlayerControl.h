#ifndef HLSPLAYERCONTROL_H
#define HLSPLAYERCONTROL_H

#include <vdr/player.h>
#include <vdr/tools.h>

#include "hlsPlayer.h"

class cHlsPlayerControl : public cControl
{
private:
	cHlsPlayer* m_pPlayer;
	std::string m_title;

public:
	cHlsPlayerControl(cHlsPlayer* Player, std::string title);
	virtual ~cHlsPlayerControl();

	virtual void Show(void);
	virtual void Hide(void);

	virtual cString GetHeader(void);
	//virtual eOSState ProcessKey(eKeys Key);

};

#endif // HLSPLAYERCONTROL_H
