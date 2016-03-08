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
	cDetailView(std::shared_ptr<skindesignerapi::cOsdView> detailView, plexclient::Video *video);
	
	void Draw();
	void Flush();
	virtual eOSState NavigateSelect();
	virtual eOSState NavigateBack();
	plexclient::Video* GetVideo() { return m_pVideo; };
	virtual void Clear();
	
private:
	std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
	std::shared_ptr<skindesignerapi::cViewElement> m_pfooter;
	std::shared_ptr<skindesignerapi::cViewElement> m_pInfo;
	std::shared_ptr<skindesignerapi::cViewElement> m_pScrollbar;
	
	plexclient::Video *m_pVideo;
	bool m_drawall;

	void DrawBackground();
	void DrawFooter();
	void DrawInfo();
	void DrawScrollbar();
};

#endif // CDETAILVIEW_H
