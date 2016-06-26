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

class cPlexSdOsd : public skindesignerapi::cSkindesignerOsdObject {
private:
    static bool m_bSdSupport;
    std::shared_ptr<cBrowserGrid> m_pBrowserGrid = nullptr;
    std::shared_ptr<cDetailView> m_pDetailGrid = nullptr;
    std::shared_ptr<skindesignerapi::cViewElement> m_pMessage = nullptr;
    bool m_messageDisplayed = false;
    bool m_detailsActive = false;
    std::shared_ptr<plexclient::MediaContainer> m_pDetailContainer = nullptr;

    std::shared_ptr<skindesignerapi::cOsdView> m_pRootView = nullptr;
    std::shared_ptr<skindesignerapi::cOsdView> m_pDetailsView = nullptr;

    void Flush();

    void DrawMessage(std::string message);

public:
    cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct);

    cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct, std::shared_ptr<plexclient::MediaContainer> detailContainer);

    ~cPlexSdOsd();

    virtual void Show(void);

    void ShowDetails(std::shared_ptr<plexclient::MediaContainer> container);
    void ShowDetails(plexclient::cVideo* vid);

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
