#ifndef CSDDISPLAYREPLAY_H
#define CSDDISPLAYREPLAY_H

#include <vdr/osdbase.h>
#include <libskindesignerapi/osdelements.h>
#include <libskindesignerapi/skindesignerosdbase.h>
#include <memory>

#include "PVideo.h"
#include "hlsPlayerControl.h"

class cSdDisplayReplay : public skindesignerapi::cSkindesignerOsdObject
{
public:
	enum eElements {
	    Background,
	    Replay,
	    Info,
		Time
	};

	cSdDisplayReplay(plexclient::Video Video, cHlsPlayerControl* Control);
	~cSdDisplayReplay();

	virtual void Show(void);
	//virtual eOSState ProcessKey(eKeys Key);

private:
	plexclient::Video m_video;
	cHlsPlayerControl* m_pContol;

	skindesignerapi::cOsdView* m_pRootView;
	std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
	std::shared_ptr<skindesignerapi::cViewElement> m_pReplay;
	std::shared_ptr<skindesignerapi::cViewElement> m_pInfo;
	std::shared_ptr<skindesignerapi::cViewElement> m_pTime;

	int m_lastsecond;

	void DrawReplay();
	void DrawInfo();
	bool DrawTime();
	void Flush();

	std::string IndexToHMS(int index, bool WithSeconds = true);
};

#endif // CSDDISPLAYREPLAY_H
