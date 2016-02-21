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
#include "tokendefinitions.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cPlexSdOsd : public skindesignerapi::cSkindesignerOsdObject
{	
private:	
	std::shared_ptr<cBrowserGrid> m_pBrowserGrid;
	std::shared_ptr<skindesignerapi::cViewElement> m_pMessage;
	bool m_messageDisplayed;

	std::shared_ptr<skindesignerapi::cOsdView> m_pRootView;
	
	void Flush();
	//void SwitchGrid(ePlexMenuTab currentTab);
	void DrawBackground();
	void DrawFooter();
	void DrawMessage(std::string message);
	
public:
	cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct);
	~cPlexSdOsd();
	virtual void Show(void);
  	virtual eOSState ProcessKey(eKeys Key);
	
	bool SdSupport();
	static cMutex RedrawMutex;
	static void DefineTokens(eViewElementsRoot ve, skindesignerapi::cTokenContainer *tk);
	static void DefineGridTokens(skindesignerapi::cTokenContainer *tk);
};

#endif // CPLEXSDOSD_H
