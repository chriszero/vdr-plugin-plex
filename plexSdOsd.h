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
#include "libskindesigner/osdelements.h"
#include "libskindesigner/skindesignerosdbase.h"

enum eViews {
	viRootView,
	viDetailView
};

enum eViewElementsRoot {
	verBackground,
	verHeader,
	verFooter
};

enum eViewGrids {
	vgBrowser
};

class cPlexSdOsd : public cSkindesignerOsdObject
{	
private:	
	std::shared_ptr<cBrowserGrid> m_pBrowserGrid;

	cOsdView* m_pRootView;
	
	void Flush();
	void SwitchGrid(ePlexMenuTab currentTab);
	void DrawBackground();
	void DrawFooter();
	
public:
	cPlexSdOsd();
	~cPlexSdOsd();
	virtual void Show(void);
  	virtual eOSState ProcessKey(eKeys Key);
	
	static cMutex RedrawMutex;
};

#endif // CPLEXSDOSD_H
