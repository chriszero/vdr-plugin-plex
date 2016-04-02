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
#include "detailView.h"
#include "tokendefinitions.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cPlexSdOsd : public skindesignerapi::cSkindesignerOsdObject
{	
private:	
	std::shared_ptr<cBrowserGrid> m_pBrowserGrid;
	std::shared_ptr<cDetailView> m_pDetailGrid;
	std::shared_ptr<skindesignerapi::cViewElement> m_pMessage;
	bool m_messageDisplayed;
	bool m_detailsActive;

	std::shared_ptr<skindesignerapi::cOsdView> m_pRootView;
	std::shared_ptr<skindesignerapi::cOsdView> m_pDetailsView;
	
	void Flush();
	void DrawMessage(std::string message);
	
	void ShowDetails(plexclient::cVideo *vid);
	
public:
	cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct);
	~cPlexSdOsd();
	virtual void Show(void);
	virtual eOSState ProcessKey(eKeys Key);
	eOSState ProcessKeyDetailView(eKeys Key);
	eOSState ProcessKeyBrowserView(eKeys Key);
	
	bool SdSupport();
	static cMutex RedrawMutex;
	static void DefineTokens(eViewElementsRoot ve, skindesignerapi::cTokenContainer *tk);
	static void DefineGridTokens(skindesignerapi::cTokenContainer *tk);
	static void DefineFooterTokens(skindesignerapi::cTokenContainer *tk);
	static void DefineWatchTokens(skindesignerapi::cTokenContainer *tk);
	static void DefineDetailsTokens(eViewElementsDetail ve, skindesignerapi::cTokenContainer *tk);
};

#endif // CPLEXSDOSD_H
