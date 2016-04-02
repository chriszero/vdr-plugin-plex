#include "plexOsd.h"
#include "plex.h"

//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////
static std::shared_ptr<plexclient::Plexservice> pPlexService;

std::shared_ptr<plexclient::Plexservice> cPlexBrowser::pLastService;
int cPlexBrowser::lastCurrentItem;

cPlexBrowser::cPlexBrowser(const char *title, std::shared_ptr<plexclient::Plexservice> Service) : cOsdMenu(title) {
    dsyslog("[plex]%s:\n", __FUNCTION__);
    pService = Service;
    //pService->Authenticate();
    if (pService == pLastService) {
        pCont = pService->GetLastSection(true);
    } else {
        pCont = pService->GetSection(pService->StartUri);
    }
    SetMenuCategory(mcRecording);
    CreateMenu();
}

cPlexBrowser *cPlexBrowser::RecoverLastState() {
    if (cPlexBrowser::pLastService != NULL) {
        cPlexBrowser *pBrowser = new cPlexBrowser("", cPlexBrowser::pLastService);
        return pBrowser;
    }
    return NULL;
}

void cPlexBrowser::CreateMenu() {
    // Clear Menu
    Clear();
    // Directory or Video?
    if (pCont && pCont->m_vDirectories.size() > 0) {

        for (std::vector<plexclient::Directory>::iterator it = pCont->m_vDirectories.begin();
             it != pCont->m_vDirectories.end(); ++it) {
            plexclient::Directory *pDir = &(*it);
            Add(new cPlexOsdItem(tr(pDir->GetTitle().c_str()), pDir));
        }
    }

    if (pCont && pCont->m_vVideos.size() > 0) {
        for (std::vector<plexclient::cVideo>::iterator it = pCont->m_vVideos.begin();
             it != pCont->m_vVideos.end(); ++it) {
            plexclient::cVideo *vid = &(*it); // cast raw pointer
            Add(new cPlexOsdItem(vid->GetTitle().c_str(), vid));
        }
    }

    if (Count() < 1) {
        Add(new cPlexOsdItem("Empty"));
    } else if (pService == pLastService) {
        // recover last selected item
        cOsdItem *item = Get(lastCurrentItem);
        SetCurrent(item);
        pLastService = NULL;
    }

    Display();
}

eOSState cPlexBrowser::ProcessKey(eKeys key) {
    eOSState state;

    // call standard function
    state = cOsdMenu::ProcessKey(key);

    int current = Current();        // get current menu item index
    cPlexOsdItem *item = static_cast<cPlexOsdItem *>(Get(current));

    if (item->IsVideo()) {
        if (item->GetAttachedVideo()->m_iViewCount > 0) SetHelp(tr("Info"), tr("Unscrobble"));
        else SetHelp(tr("Info"), tr("Scrobble"));
    } else {
        SetHelp(NULL);
    }

    switch (state) {
        case osUnknown:
            switch (key) {
                case kOk:
                    return ProcessSelected();
                case kBack:
                    return LevelUp();
                case kRed:
                    if (item->IsVideo()) {
                        std::cout << " Video Info";
                    }
                    std::cout << std::endl;
                    break;
                case kGreen:
                    if (item->IsVideo()) {
                        if (item->GetAttachedVideo()->m_iViewCount > 0) {
                            if (item->GetAttachedVideo()->SetUnwatched()) {
                                item->GetAttachedVideo()->UpdateFromServer();
                            }
                        } else {
                            if (item->GetAttachedVideo()->SetWatched()) {
                                item->GetAttachedVideo()->UpdateFromServer();
                            }
                        }
                    }
                    if (item->GetAttachedVideo()->m_iViewCount > 0) SetHelp(tr("Info"), tr("Unscrobble"));
                    else SetHelp(tr("Info"), tr("Scrobble"));
                    break;
                default:
                    break;
            }
            break;
        case osBack:
            state = LevelUp();
            if (state == osEnd) {    // top level reached
                return osPlugin;
            }
        default:
            break;
    }
    return state;
}

eOSState cPlexBrowser::LevelUp() {
    pCont = pService->GetLastSection();
    if (!pCont) {
        cPlexMenu::eShow = menuShow::MAIN;
        return osEnd;
    }
    cString title = cString::sprintf(tr("Browse Plex - %s"), tr(pCont->m_sTitle1.c_str()));
    SetTitle(title);
    CreateMenu();
    return osContinue;
}

eOSState cPlexBrowser::ProcessSelected() {
    int current = Current();        // get current menu item index
    cPlexOsdItem *item = static_cast<cPlexOsdItem *>(Get(current));


    if (item->IsVideo()) {
        pLastService = pService;
        lastCurrentItem = current;
        cMyPlugin::PlayFile(*item->GetAttachedVideo());
        return osEnd;
    }

    if (item->IsDir()) {
        plexclient::Directory *pDir = item->GetAttachedDirectory();
        pCont = pService->GetSection(pDir->m_sKey);
        cString title = cString::sprintf(tr("Browse Plex - %s"), tr(pDir->m_sTitle.c_str()));
        SetTitle(title);
        CreateMenu();
        return osContinue;
    }

    //return osEnd;
    return osContinue;
}


//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////

menuShow cPlexMenu::eShow = MAIN;

/**
**	Plex menu constructor.
*/
cPlexMenu::cPlexMenu(const char *title, int c0, int c1, int c2, int c3, int c4)
        : cOsdMenu(title) {
    SetHasHotkeys();

    for (std::vector<plexclient::PlexServer>::iterator it = plexclient::plexgdm::GetInstance().GetPlexservers().begin();
         it != plexclient::plexgdm::GetInstance().GetPlexservers().end(); ++it) {
        for (std::vector<ViewEntry>::iterator vEntry = Config::GetInstance().m_serverViewentries.begin();
             vEntry != Config::GetInstance().m_serverViewentries.end(); ++vEntry) {
            auto plexService = std::make_shared<plexclient::Plexservice>(&(*it));
            plexService->StartUri = vEntry->PlexPath;
            Add(new cPlexOsdItem(Poco::format("%s - %s", it->GetServerName(), vEntry->Name).c_str(), plexService));
        }
        for (std::vector<ViewEntry>::iterator vEntry = Config::GetInstance().m_viewentries.begin();
             vEntry != Config::GetInstance().m_viewentries.end(); ++vEntry) {
            auto plexService = std::make_shared<plexclient::Plexservice>(&(*it));
            plexService->StartUri = vEntry->PlexPath;
            Add(new cPlexOsdItem(Poco::format("%s - %s", it->GetServerName(), vEntry->Name).c_str(), plexService));
        }
    }


    if (Count() < 1) {
        Add(new cPlexOsdItem(tr("No Plex Media Server found.")), false);
    }
}

cOsdMenu *cPlexMenu::ProcessMenu() {
    if (cMyPlugin::CalledFromCode) {
        cMyPlugin::CalledFromCode = false;
        return cPlexBrowser::RecoverLastState();
    }

    if (cPlexMenu::eShow == menuShow::BROWSER) {
        return new cPlexBrowser(tr("Browse Plex"), pPlexService);
    }
    return new cPlexMenu("Plex");
}


/**
**	Handle play plugin menu key event.
**
**	@param key	key event
*/
eOSState cPlexMenu::ProcessKey(eKeys key) {
    eOSState state;

    //if (key != kNone) {
    //	dsyslog("[plex]%s: key=%d\n", __FUNCTION__, key);
    //}
    // call standard function
    state = cOsdMenu::ProcessKey(key);

    int current = Current();        // get current menu item index
    cPlexOsdItem *item = static_cast<cPlexOsdItem *>(Get(current));

    switch (state) {
        case osUnknown:
            switch (key) {
                case kOk:
                    pPlexService = item->GetAttachedService();
                    cPlexMenu::eShow = menuShow::BROWSER;
                    return osPlugin;        // restart with OSD browser
                default:
                    break;
            }
        default:
            break;
    }
    return state;
}


