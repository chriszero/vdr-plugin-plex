#include "browserGrid.h"
#include "plexSdOsd.h"
#include "XmlObject.h"
#include "PVideo.h"
#include "Directory.h"
#include "plex.h"
#include "pictureCache.h"
#include "tokendefinitions.h"

cBrowserGrid::cBrowserGrid(std::shared_ptr<skindesignerapi::cOsdView> rootView)
        : cViewGridNavigator(rootView, std::shared_ptr<skindesignerapi::cViewElement>(
        rootView->GetViewElement((int) eViewElementsRoot::scrollbar))),
          cSdClock(std::shared_ptr<skindesignerapi::cViewElement>(
                  rootView->GetViewElement((int) eViewElementsRoot::watch))) {
    m_pBackground = std::shared_ptr<skindesignerapi::cViewElement>(
            rootView->GetViewElement((int) eViewElementsRoot::background));
    m_pHeader = std::shared_ptr<skindesignerapi::cViewElement>(
            rootView->GetViewElement((int) eViewElementsRoot::header));
    m_pfooter = std::shared_ptr<skindesignerapi::cViewElement>(
            rootView->GetViewElement((int) eViewElementsRoot::footer));
    m_pInfopane = std::shared_ptr<skindesignerapi::cViewElement>(
            rootView->GetViewElement((int) eViewElementsRoot::infopane));

    m_pService = NULL;
    m_pContainer = NULL;
    m_viewEntryIndex = 0;
    m_redrawBackground = true;

    Config *conf = &Config::GetInstance();
    if (conf->DefaultViewMode == ViewMode::Cover) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid((int) eViewGrids::cover)));
        SetGridDimensions(conf->CoverGridRows, conf->CoverGridColumns);
    } else if (conf->DefaultViewMode == ViewMode::Detail) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid((int) eViewGrids::detail)));
        SetGridDimensions(conf->DetailGridRows, conf->DetailGridColumns);
    } else if (conf->DefaultViewMode == ViewMode::List) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid((int) eViewGrids::list)));
        SetGridDimensions(conf->ListGridRows, conf->ListGridColumns);
    }

    SwitchGrid(m_viewEntryIndex);
    SwitchView();
}

cBrowserGrid::~cBrowserGrid() {
    m_vServerElements.clear();
    m_vElements.clear();
}

void cBrowserGrid::Clear() {
    m_pBackground->Clear();
    m_pfooter->Clear();
    m_pInfopane->Clear();
    m_pWatch->Clear();
    m_pGrid->Clear();
}

void cBrowserGrid::Flush() {
    if (m_redrawBackground) {
        m_pBackground->Display();
        m_redrawBackground = false;
    }

    cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
    m_pGrid->Display();
    m_pScrollbar->Display();

    m_pRootView->Display();
}

void cBrowserGrid::SwitchView(ViewMode mode) {
    auto selObj = SelectedObject();
    if (!selObj) return;

    Config *conf = &Config::GetInstance();
    conf->DefaultViewMode = mode;
    if (conf->DefaultViewMode == ViewMode::Cover) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid((int) eViewGrids::cover)));
        SetGridDimensions(conf->CoverGridRows, conf->CoverGridColumns);
    } else if (conf->DefaultViewMode == ViewMode::Detail) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid((int) eViewGrids::detail)));
        SetGridDimensions(conf->DetailGridRows, conf->DetailGridColumns);
    } else if (conf->DefaultViewMode == ViewMode::List) {
        SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid((int) eViewGrids::list)));
        SetGridDimensions(conf->ListGridRows, conf->ListGridColumns);
    }

    int activePos = selObj->AbsolutePosition;
    //ProcessData();

    for (std::vector<cGridElement *>::iterator it = m_vElements.begin(); it != m_vElements.end(); ++it) {
        cGridElement *elem = *it;
        elem->Position = -1;
        elem->Dirty();
        elem->SetPosition(-1, -1);
    }

    m_pGrid->Clear();
    m_startIndex = activePos;
    m_setIterator = true;
    FilterElements(0);
}

void cBrowserGrid::ShowDirectory(std::shared_ptr<plexclient::MediaContainer> container){
    m_pContainer = container;
    m_bServersAreRoot = false;
    m_vServerElements.clear();
    ProcessData();
}

void cBrowserGrid::NextViewMode() {
    ViewMode mode = Config::GetInstance().DefaultViewMode;
    if (mode == ViewMode::Cover) {
        mode = ViewMode::Detail;
    } else if (mode == ViewMode::Detail) {
        mode = ViewMode::List;
    } else if (mode == ViewMode::List) {
        mode = ViewMode::Cover;
    }
    SwitchView(mode);
}

void cBrowserGrid::SwitchGrid(int index) {
    m_pHeader->Clear();
    m_pHeader->ClearTokens();
    m_pHeader->AddIntToken((int) eTokenGridInt::columns, m_columns);
    m_pHeader->AddIntToken((int) eTokenGridInt::rows, m_rows);
    m_pHeader->AddIntToken((int) eTokenGridInt::totalcount, m_vElements.size());

    if (plexclient::plexgdm::GetInstance().GetFirstServer()) {
        if (m_viewEntryIndex < (int) Config::GetInstance().m_viewentries.size()) {
            ViewEntry entry = Config::GetInstance().m_viewentries[index];
            m_pHeader->AddStringToken((int) eTokenGridStr::tabname, tr(entry.Name.c_str()));
            m_pService = std::shared_ptr<plexclient::Plexservice>(
                    new plexclient::Plexservice(plexclient::plexgdm::GetInstance().GetFirstServer(), entry.PlexPath));
            m_pContainer = m_pService->GetSection(m_pService->StartUri);
            m_bServersAreRoot = false;
            m_vServerElements.clear();
        } else {
            //Server View
            m_pHeader->AddStringToken((int) eTokenGridStr::tabname, tr("Library"));
            m_pService = NULL;
            m_pContainer = NULL;
            m_bServersAreRoot = true;
            SetServerElements();
        }
    } else {
        m_pHeader->AddStringToken((int) eTokenGridStr::tabname, tr("No Plex Media Server found."));
        m_pInfopane->AddStringToken((int) eTokenGridStr::title, tr("No Plex Media Server found."));
        m_pService = NULL;
        m_pContainer = NULL;
    }
    ProcessData();
    auto selObj = SelectedObject();
    if (selObj) {
        selObj->AddTokens(m_pHeader, false);
        m_pHeader->AddIntToken((int) eTokenGridInt::position, selObj->AbsolutePosition);
    }

    DrawBackground();
}

void cBrowserGrid::SetServerElements() {
    m_vServerElements.clear();

    for (std::vector<plexclient::PlexServer>::iterator it = plexclient::plexgdm::GetInstance().GetPlexservers().begin();
         it != plexclient::plexgdm::GetInstance().GetPlexservers().end(); ++it) {
        for (std::vector<ViewEntry>::iterator vEntry = Config::GetInstance().m_serverViewentries.begin();
             vEntry != Config::GetInstance().m_serverViewentries.end(); ++vEntry) {
            m_vServerElements.push_back(cServerElement(&(*it), vEntry->PlexPath, vEntry->Name));
        }
    }
}

void cBrowserGrid::ProcessData() {
    m_vElements.clear();

    if (m_vServerElements.size() > 0) {
        for (auto it = m_vServerElements.begin(); it != m_vServerElements.end(); ++it) {
            cServerElement *elem = &(*it);
            m_vElements.push_back(elem);
        }
    } else if (m_pContainer) {
        if (!m_pService->IsRoot()) {
            m_vElements.push_back(&m_Dummy);
            m_Dummy.Dirty();
        }

        if (m_pContainer->m_vVideos.size() > 0) {
            for (std::vector<plexclient::cVideo>::iterator it = m_pContainer->m_vVideos.begin();
                 it != m_pContainer->m_vVideos.end(); ++it) {
                plexclient::cVideo *elem = &(*it);
                m_vElements.push_back(elem);
            }
        }
        if (m_pContainer->m_vDirectories.size() > 0) {
            for (std::vector<plexclient::Directory>::iterator it = m_pContainer->m_vDirectories.begin();
                 it != m_pContainer->m_vDirectories.end(); ++it) {
                plexclient::Directory *elem = &(*it);
                m_vElements.push_back(elem);
            }
        }
    }

    int pos = 0;
    for (std::vector<cGridElement *>::iterator it = m_vElements.begin(); it != m_vElements.end(); ++it) {
        cGridElement *elem = *it;
        elem->AbsolutePosition = pos++;
    }

    m_startIndex = 0;

    m_pGrid->Clear();
    m_setIterator = true;
    FilterElements(0);
}

eOSState cBrowserGrid::NavigateSelect() {
    if (m_setIterator) return eOSState::osContinue;;
    plexclient::Directory *dir = dynamic_cast<plexclient::Directory *>(SelectedObject());
    if (dir) {
        m_pContainer = m_pService->GetSection(dir->m_sKey);
        ProcessData();
        return eOSState::osContinue;
    } else if (dynamic_cast<cDummyElement *>(SelectedObject())) {
        return NavigateBack();
    } else if (cServerElement *srv = dynamic_cast<cServerElement *>(SelectedObject())) {
        m_pService = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice(srv->Server()));
        m_pContainer = m_pService->GetSection(srv->StartPath());
        m_vServerElements.clear();
        ProcessData();
        return eOSState::osContinue;
    } else if (dynamic_cast<plexclient::cVideo *>(SelectedObject())) {
        return eOSState::osUser1;
    } else return eOSState::osEnd;
}

eOSState cBrowserGrid::NavigateBack() {
    if (m_setIterator) return eOSState::osContinue;;
    std::shared_ptr<plexclient::MediaContainer> pCont = NULL;
    if (m_pService) {
        pCont = m_pService->GetLastSection();
    }

    if (pCont) {
        m_pContainer = pCont;
        ProcessData();
        return eOSState::osContinue;

    } else if (m_bServersAreRoot && m_pService) {
        m_pContainer = NULL;
        m_pService = NULL;
        SetServerElements();
        ProcessData();
        return eOSState::osContinue;
    }
    return eOSState::osEnd;
}

void cBrowserGrid::DrawGrid() {
    m_pHeader->Display();
    DrawInfopane();
    DrawFooter();
    DrawScrollbar();
}

void cBrowserGrid::DrawBackground() {
    m_redrawBackground = true;
    m_pBackground->ClearTokens();

    /*auto video = dynamic_cast<plexclient::Video*>(SelectedObject());
    if(video) {
        bool cached = false;
        std::string path = cPictureCache::GetInstance().GetPath(video->ArtUri(), 1920, 1080, cached);
        m_pBackground->AddStringToken("selecteditembackground", path);
    }
    */
    m_pBackground->AddIntToken((int) eTokenBackgroundInt::isdirectory, 1);
    m_pBackground->AddStringToken((int) eTokenBackgroundStr::currentdirectorybackground, "/path");
    m_pBackground->AddIntToken((int) eTokenBackgroundInt::viewmode, Config::GetInstance().DefaultViewMode);
}

void cBrowserGrid::DrawInfopane() {
    m_pInfopane->Clear();
    if (SelectedObject()) {
        SelectedObject()->AddTokens(m_pInfopane, true);
        m_pInfopane->AddIntToken((int) eTokenGridInt::columns, m_columns);
        m_pInfopane->AddIntToken((int) eTokenGridInt::rows, m_rows);
        m_pInfopane->AddIntToken((int) eTokenGridInt::totalcount, m_vElements.size());
        m_pInfopane->AddIntToken((int) eTokenGridInt::position, SelectedObject()->AbsolutePosition);
    }
    m_pInfopane->Display();
}

void cBrowserGrid::DrawFooter() {
    //if (!active)
    //   return;
    cString nextTab = "Library";
    if (m_viewEntryIndex + 1 < (int) Config::GetInstance().m_viewentries.size()) {
        nextTab = Config::GetInstance().m_viewentries[m_viewEntryIndex + 1].Name.c_str();
    } else if (m_viewEntryIndex + 1 == (int) Config::GetInstance().m_viewentries.size() + 1) {
        nextTab = Config::GetInstance().m_viewentries[0].Name.c_str();
    }
    cString details = "Details";

    string textGreen = tr(details);
    string textYellow = tr(nextTab);
    string textRed = "";
    string textBlue = tr("Switch View");

    if (auto vid = dynamic_cast<plexclient::cVideo *>(SelectedObject())) {
        if (vid->m_iViewCount > 0) textRed = tr("Unscrobble");
        else textRed = tr("Scrobble");
    }


    int colorKeys[4] = {Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3};

    m_pfooter->Clear();
    m_pfooter->ClearTokens();

    m_pfooter->AddStringToken((int) eTokenFooterStr::red, textRed.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::green, textGreen.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::yellow, textYellow.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::blue, textBlue.c_str());

    for (int button = 0; button < 4; button++) {
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button]) {
            case 0:
                isRed = true;
                break;
            case 1:
                isGreen = true;
                break;
            case 2:
                isYellow = true;
                break;
            case 3:
                isBlue = true;
                break;
            default:
                break;
        }
        m_pfooter->AddIntToken(0 + button, isRed);
        m_pfooter->AddIntToken(4 + button, isGreen);
        m_pfooter->AddIntToken(8 + button, isYellow);
        m_pfooter->AddIntToken(12 + button, isBlue);
    }

    m_pfooter->Display();
}

void cBrowserGrid::NextTab() {
    m_viewEntryIndex++;
    if (m_viewEntryIndex > (int) Config::GetInstance().m_viewentries.size()) {
        m_viewEntryIndex = 0;
    }
    SwitchGrid(m_viewEntryIndex);
}

void cBrowserGrid::PrevTab() {
    m_viewEntryIndex--;
    if (m_viewEntryIndex < 0) {
        m_viewEntryIndex = Config::GetInstance().m_viewentries.size();
    }
    SwitchGrid(m_viewEntryIndex);
}

/*
 * cDummyElement
 */

cDummyElement::cDummyElement() {
    m_title = "../";
}

cDummyElement::cDummyElement(std::string title) {
    m_title = title;
}

void cDummyElement::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear,
                              std::function<void(cGridElement *)> OnCached) {
    if (clear) grid->ClearTokens();
    grid->AddIntToken((int) eTokenGridInt::isdummy, 1);
    grid->AddStringToken((int) eTokenGridStr::title, m_title.c_str());
}

std::string cDummyElement::GetTitle() {
    return "Dummy";
}

/*
 * cServerElement
 */

cServerElement::cServerElement(plexclient::PlexServer *server, std::string startPath, std::string startName) {
    m_pServer = server;
    m_sStartPath = startPath;
    m_sStartName = startName;
}

void cServerElement::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear,
                               std::function<void(cGridElement *)> OnCached) {
    if (clear) grid->ClearTokens();
    grid->AddIntToken((int) eTokenGridInt::isserver, 1);
    grid->AddStringToken((int) eTokenGridStr::title, m_pServer->GetServerName().c_str());
    grid->AddStringToken((int) eTokenGridStr::serverstartpointname, m_sStartName.c_str());
    grid->AddStringToken((int) eTokenGridStr::serverip, m_pServer->GetHost().c_str());
    grid->AddIntToken((int) eTokenGridInt::serverport, m_pServer->GetPort());
    grid->AddStringToken((int) eTokenGridStr::serverversion, m_pServer->GetVersion().c_str());
    grid->AddIntToken((int) eTokenGridInt::viewmode, Config::GetInstance().DefaultViewMode);
}

std::string cServerElement::GetTitle() {
    return "Server";
}
