#ifndef CPLEXSDOSD_H
#define CPLEXSDOSD_H

#include <vdr/osdbase.h>

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>

#include "Config.h"
#include "Plexservice.h"

#include "plexgdm.h"
#include "hlsPlayerControl.h"

#include "browserGrid.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

enum eViews {
	viRootView,
	viDetailView
};

enum eViewElementsRoot {
	verBackground,
	verHeader,
	verFooter,
	verInfopane,
	verWatch,
	verMessage,
	verScrollbar
};

enum eViewGrids {
	vgCover,
	vgDetail,
	vgList
};

enum eViewElementsDetail {
    vedBackground,
    vedHeader,
    vedFooter
};

class cPlexSdOsd : public skindesignerapi::cSkindesignerOsdObject
{	
private:	
	std::shared_ptr<cBrowserGrid> m_pBrowserGrid;
	std::shared_ptr<skindesignerapi::cViewElement> m_pMessage;
	bool m_messageDisplayed;

	skindesignerapi::cOsdView* m_pRootView;
	
	void Flush();
	//void SwitchGrid(ePlexMenuTab currentTab);
	void DrawBackground();
	void DrawFooter();
	void DrawMessage(std::string message);
	
public:
	cPlexSdOsd();
	~cPlexSdOsd();
	virtual void Show(void);
  	virtual eOSState ProcessKey(eKeys Key);
	
	bool SdSupport();
	static cMutex RedrawMutex;
};

#endif // CPLEXSDOSD_H
