#ifndef CDETAILVIEW_H
#define CDETAILVIEW_H

#include "viewGridNavigator.h"
#include "PVideo.h"

#include "tokendefinitions.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cDetailView : public cViewGridNavigator
{
public:
	cDetailView(std::shared_ptr<skindesignerapi::cOsdView> detailView, plexclient::cVideo *video);
	
	void Draw();
	virtual void Flush();
	virtual eOSState NavigateSelect();
	virtual eOSState NavigateBack();
	plexclient::cVideo* GetVideo() { return m_pVideo; };
	virtual void Clear();
	bool DrawTime();
	
private:
	std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
	std::shared_ptr<skindesignerapi::cViewElement> m_pfooter;
	std::shared_ptr<skindesignerapi::cViewElement> m_pInfo;
	std::shared_ptr<skindesignerapi::cViewElement> m_pScrollbar;
	std::shared_ptr<skindesignerapi::cViewElement> m_pWatch;
	
	plexclient::cVideo *m_pVideo;
	bool m_drawall;
	int m_lastsecond;

	void DrawBackground();
	void DrawFooter();
	void DrawInfo();
	void DrawScrollbar();
};

#endif // CDETAILVIEW_H
