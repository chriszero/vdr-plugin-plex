#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"
#include "plexOsd.h"
#include "services.h"
#include "tokendefinitions.h"

#include <Poco/Net/SSLManager.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/ConsoleCertificateHandler.h>

//////////////////////////////////////////////////////////////////////////////
//	cPlugin
//////////////////////////////////////////////////////////////////////////////

volatile bool cMyPlugin::CalledFromCode = false;

#ifdef SKINDESIGNER
bool cMyPlugin::bSkindesigner = false;
#endif

plexclient::cVideo cMyPlugin::CurrentVideo;
bool cMyPlugin::PlayingFile = false;

/**
**	Initialize any member variables here.
**
**	@note DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
**	VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
*/
cMyPlugin::cMyPlugin(void) {
}

/**
**	Clean up after yourself!
*/
cMyPlugin::~cMyPlugin(void) {
    plexclient::plexgdm::GetInstance().stopRegistration();
    plexclient::ControlServer::GetInstance().Stop();
}

/**
**	Return plugin version number.
**
**	@returns version number as constant string.
*/
const char *cMyPlugin::Version(void) {
    return VERSION;
}

/**
**	Return plugin short description.
**
**	@returns short description as constant string.
*/
const char *cMyPlugin::Description(void) {
    return DESCRIPTION;
}


bool cMyPlugin::Start(void) {
#ifdef SKINDESIGNER
    m_pPlugStruct = new skindesignerapi::cPluginStructure();
    m_pPlugStruct->name = "plex";
    m_pPlugStruct->libskindesignerAPIVersion = LIBSKINDESIGNERAPIVERSION;

    m_pPlugStruct->RegisterRootView("root.xml");
    skindesignerapi::cTokenContainer *tkBackground = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::background, tkBackground);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::background, "background",
                                       tkBackground);

    skindesignerapi::cTokenContainer *tkHeader = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::header, tkHeader);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::header, "header", tkHeader);

    skindesignerapi::cTokenContainer *tkFooter = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::footer, tkFooter);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::footer, "footer", tkFooter);

    skindesignerapi::cTokenContainer *tkInfopane = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::infopane, tkInfopane);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::infopane, "infopane",
                                       tkInfopane);

    skindesignerapi::cTokenContainer *tkWatch = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::watch, tkWatch);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::watch, "time", tkWatch);

    skindesignerapi::cTokenContainer *tkMessage = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::message, tkMessage);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::message, "message", tkMessage);

    skindesignerapi::cTokenContainer *tkScrollbar = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineTokens(eViewElementsRoot::scrollbar, tkScrollbar);
    m_pPlugStruct->RegisterViewElement((int) eViews::rootView, (int) eViewElementsRoot::scrollbar, "scrollbar",
                                       tkScrollbar);

    skindesignerapi::cTokenContainer *tkGridCover = new skindesignerapi::cTokenContainer();
    skindesignerapi::cTokenContainer *tkGridDetail = new skindesignerapi::cTokenContainer();
    skindesignerapi::cTokenContainer *tkGridList = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineGridTokens(tkGridCover);
    cPlexSdOsd::DefineGridTokens(tkGridDetail);
    cPlexSdOsd::DefineGridTokens(tkGridList);
    m_pPlugStruct->RegisterViewGrid((int) eViews::rootView, (int) eViewGrids::cover, "coverbrowser", tkGridCover);
    m_pPlugStruct->RegisterViewGrid((int) eViews::rootView, (int) eViewGrids::detail, "detailbrowser", tkGridDetail);
    m_pPlugStruct->RegisterViewGrid((int) eViews::rootView, (int) eViewGrids::list, "listbrowser", tkGridList);

    // DetailsView
    m_pPlugStruct->RegisterSubView((int) eViews::detailView, "detail.xml");
    skindesignerapi::cTokenContainer *tkBackgroundDetail = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail::background, tkBackgroundDetail);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::background, "background",
                                       tkBackgroundDetail);

    skindesignerapi::cTokenContainer *tkDetailInfo = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail::info, tkDetailInfo);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::info, "info", tkDetailInfo);

    skindesignerapi::cTokenContainer *tkDetailFooter = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail::footer, tkDetailFooter);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::footer, "footer",
                                       tkDetailFooter);

    skindesignerapi::cTokenContainer *tkDetailScrollbar = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail::scrollbar, tkDetailScrollbar);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::scrollbar, "scrollbar",
                                       tkDetailScrollbar);

    skindesignerapi::cTokenContainer *tkDetailMessage = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail::message, tkDetailMessage);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::message, "message",
                                       tkDetailMessage);

    skindesignerapi::cTokenContainer *tkDetailExtraGrid = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineGridTokens(tkDetailExtraGrid);
    m_pPlugStruct->RegisterViewGrid((int) eViews::detailView, (int) eViewDetailViewGrids::extras, "extragrid",
                                    tkDetailExtraGrid);

    skindesignerapi::cTokenContainer *tkDetailWatch = new skindesignerapi::cTokenContainer();
    cPlexSdOsd::DefineWatchTokens(tkDetailWatch);
    m_pPlugStruct->RegisterViewElement((int) eViews::detailView, (int) eViewElementsDetail::watch, "time",
                                       tkDetailWatch);


    if (!skindesignerapi::SkindesignerAPI::RegisterPlugin(m_pPlugStruct)) {
        esyslog("[plex]: skindesigner not available");
        bSkindesigner = false;
    } else {
        dsyslog("[plex]: successfully registered at skindesigner, id %d", m_pPlugStruct->id);
        m_pTestOsd = new cPlexSdOsd(m_pPlugStruct);
        bSkindesigner = true;
    }
#endif
    return true;
}

/**
**	Start any background activities the plugin shall perform.
*/
bool cMyPlugin::Initialize(void) {
    // Initialize SSL
    {
        using namespace Poco;
        using namespace Poco::Net;
        using Poco::Net::SSLManager;
        using Poco::Net::Context;
        using Poco::Net::AcceptCertificateHandler;
        using Poco::Net::PrivateKeyPassphraseHandler;
        using Poco::Net::InvalidCertificateHandler;
        using Poco::Net::ConsoleCertificateHandler;

        //SharedPtr<PrivateKeyPassphraseHandler> pConsoleHandler = new PrivateKeyPassphraseHandler;
        SharedPtr<InvalidCertificateHandler> pInvalidCertHandler = new AcceptCertificateHandler(false);
        Context::Ptr pContext = new Poco::Net::Context(
                Context::CLIENT_USE, "", "", "", Context::VERIFY_NONE, // VERIFY_NONE...?!
                9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        SSLManager::instance().initializeClient(NULL, pInvalidCertHandler, pContext);
    }
    // First Startup? Save UUID
    SetupStore("UUID", Config::GetInstance().GetUUID().c_str());

    plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(),
                                                     Config::GetInstance().GetHostname(), "3200", DESCRIPTION, VERSION);
    plexclient::plexgdm::GetInstance().Start();
    plexclient::ControlServer::GetInstance().Start();

    return true;
}

/**
**	Create main menu entry.
*/
const char *cMyPlugin::MainMenuEntry(void) {
    return Config::GetInstance().HideMainMenuEntry ? NULL : MAINMENUENTRY;
}

/**
**	Perform the action when selected from the main VDR menu.
*/
cOsdObject *cMyPlugin::MainMenuAction(void) {
    //dsyslog("[plex]%s:\n", __FUNCTION__);
#ifdef SKINDESIGNER
    if (bSkindesigner && m_pTestOsd->SdSupport()) {
        if (m_bShowInfo) {
            m_bShowInfo = false;
            return new cPlexSdOsd(m_pPlugStruct, action.container);
        }
        return new cPlexSdOsd(m_pPlugStruct);
    }
    else return cPlexMenu::ProcessMenu();
#else
    return cPlexMenu::ProcessMenu();
#endif
}

/**
**	Called for every plugin once during every cycle of VDR's main program
**	loop.
*/
void cMyPlugin::MainThreadHook(void) {
    // dsyslog("[plex]%s:\n", __FUNCTION__);
    // Start Tasks, e.g. Play Video
    using namespace plexclient;
    if (ActionManager::GetInstance().IsAction()) {
        action = ActionManager::GetInstance().GetAction();
        if(action.type == ActionType::Play) {
            if(action.container->m_vVideos.size() > 0)
                PlayFile(action.container->m_vVideos[0]);
        }
        else if (bSkindesigner && action.type == ActionType::Display) {
            m_bShowInfo = true;
            cRemote::CallPlugin("plex");
        }
    }

}

/**
**	Return our setup menu.
*/
cMenuSetupPage *cMyPlugin::SetupMenu(void) {
    //dsyslog("[plex]%s:\n", __FUNCTION__);

    return new cMyMenuSetupPage;
}

/**
**	Parse setup parameters
**
**	@param name	paramter name (case sensetive)
**	@param value	value as string
**
**	@returns true if the parameter is supported.
*/
bool cMyPlugin::SetupParse(const char *name, const char *value) {
    //dsyslog("[plex]%s: '%s' = '%s'\n", __FUNCTION__, name, value);

    return Config::GetInstance().Parse(name, value);
}

/**
**	Play a file.
**
**	@param filename	path and file name
*/
void cMyPlugin::PlayFile(plexclient::cVideo Vid) {
    if (Vid.m_iMyPlayOffset == 0 && Vid.m_lViewoffset > 0) {
        cString message = cString::sprintf(tr("To start from %ld minutes, press Ok."), Vid.m_lViewoffset / 60000);
        eKeys response = Skins.Message(eMessageType::mtInfo, message, 5);
        if (response == kOk) {
            Vid.m_iMyPlayOffset = Vid.m_lViewoffset / 1000;
        }
    }

    cPlugin *mpvPlugin = cPluginManager::GetPlugin("mpv");

    if (Config::GetInstance().UseMpv && mpvPlugin) {
        CurrentVideo = Vid;
        cMyPlugin::PlayingFile = true;

        Mpv_PlayFile req;
        Mpv_SetTitle reqTitle;
        std::string file;
        if (Config::GetInstance().UsePlexAccount && Vid.m_pServer->IsLocal() == false) {
            file = plexclient::Plexservice::GetUniversalTranscodeUrl(&Vid, Vid.m_iMyPlayOffset, NULL, true);
        }
        else {
            file = (Vid.m_pServer->GetUri() + Vid.m_Media.m_sPartKey);
        }

        req.Filename = (char *) file.c_str();
        mpvPlugin->Service(MPV_PLAY_FILE, &req);

        reqTitle.Title = (char *) Vid.GetTitle().c_str();
        mpvPlugin->Service(MPV_SET_TITLE, &reqTitle);

        // Set "StartAt" for mpv player
        Mpv_Seek seekData;
        seekData.SeekAbsolute = Vid.m_iMyPlayOffset;
        seekData.SeekRelative = 0;
        mpvPlugin->Service(MPV_SEEK, &seekData);

        return;

    } else if (Config::GetInstance().UseMpv) {
        isyslog("Can't find mpv %s, playing directly.", mpvPlugin ? "service" : "plugin");
    }


    isyslog("[plex]: play file '%s'\n", Vid.m_sKey.c_str());
    cControl *control = cHlsPlayerControl::Create(Vid);
    if (control) {
        cControl::Launch(control);
    }
}

VDRPLUGINCREATOR(cMyPlugin);        // Don't touch this!
