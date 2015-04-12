#include "browserGrid.h"
#include "plexSdOsd.h"
#include "XmlObject.h"
#include "PVideo.h"
#include "Directory.h"
#include "plex.h"
#include "pictureCache.h"

cBrowserGrid::cBrowserGrid(skindesignerapi::cOsdView* rootView) : cViewGridNavigator(rootView, rootView->GetViewGrid(eViewGrids::vgBrowser) )
{
	m_pBackground = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verBackground));
	m_pViewHeader = std::shared_ptr<cViewHeader>( new cViewHeader(rootView->GetViewElement(eViewElementsRoot::verHeader)));
	m_pfooter = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verFooter));
	m_pInfopane = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verInfopane));

	m_rows = Config::GetInstance().GridRows;
	m_columns = Config::GetInstance().GridColumns;
	m_pService = NULL;
	m_pContainer = NULL;
	SwitchGrid(m_pViewHeader->CurrentTab());
}

cBrowserGrid::~cBrowserGrid()
{
	m_vServerElements.clear();
	m_vElements.clear();
}

void cBrowserGrid::Flush() 
{ 
	cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
	m_pBackground->Display();
	m_pInfopane->Display();
	m_pGrid->Display();
	m_pRootView->Display();
}

void cBrowserGrid::SwitchGrid(ePlexMenuTab currentTab)
{
	cPictureCache::GetInstance().RemoveAll();
	if(currentTab == ePlexMenuTab::pmtOnDeck) {
		m_pService = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( plexclient::plexgdm::GetInstance().GetFirstServer(), "/library/onDeck" ) );
		m_pContainer = m_pService->GetSection(m_pService->StartUri);
		m_bServersAreRoot = false;
		m_vServerElements.clear();
		ProcessData();

	} else if(currentTab == ePlexMenuTab::pmtRecentlyAdded) {
		m_pService = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( plexclient::plexgdm::GetInstance().GetFirstServer(), "/library/recentlyAdded" ) );
		m_pContainer = m_pService->GetSection(m_pService->StartUri);
		m_bServersAreRoot = false;
		m_vServerElements.clear();
		ProcessData();

	} else if(currentTab == ePlexMenuTab::pmtLibrary) {
		//Server View
		m_pService = NULL;
		m_pContainer = NULL;
		m_bServersAreRoot = true;
		SetServerElements();
		ProcessData();
	}
}

void cBrowserGrid::SetServerElements()
{
	m_vServerElements.clear();

	for(std::vector<plexclient::PlexServer>::iterator it = plexclient::plexgdm::GetInstance().GetPlexservers().begin(); it != plexclient::plexgdm::GetInstance().GetPlexservers().end(); ++it) {
		m_vServerElements.push_back(cServerElement(&(*it), "/library/sections", "Bibliothek"));
		m_vServerElements.push_back(cServerElement(&(*it), "/video", "Video Channels"));
	}
}

void cBrowserGrid::ProcessData()
{
	m_vElements.clear();

	if(m_vServerElements.size() > 0) {
		for(auto it = m_vServerElements.begin(); it !=m_vServerElements.end(); ++it) {
			cServerElement *elem = &(*it);
			m_vElements.push_back(elem);
		}
	} else if (m_pContainer) {
		if (!m_pService->IsRoot()) {
			m_vElements.push_back(&m_Dummy);
			m_Dummy.Dirty();
		}

		if(m_pContainer->m_vVideos.size() > 0) {
			for(std::vector<plexclient::Video>::iterator it = m_pContainer->m_vVideos.begin(); it != m_pContainer->m_vVideos.end(); ++it) {
				plexclient::Video *elem = &(*it);
				m_vElements.push_back(elem);
			}
		}
		if(m_pContainer->m_vDirectories.size() > 0) {
			for(std::vector<plexclient::Directory>::iterator it = m_pContainer->m_vDirectories.begin(); it != m_pContainer->m_vDirectories.end(); ++it) {
				plexclient::Directory *elem = &(*it);
				m_vElements.push_back(elem);
			}
		}
	}

	m_firstElementIter = m_vElements.begin();

	m_pGrid->Clear();
	m_setIterator = true;
	FilterElements(0);
}

eOSState cBrowserGrid::NavigateSelect()
{
	plexclient::Directory* dir = dynamic_cast<plexclient::Directory*>(SelectedObject());
	if(dir) {
		m_pContainer = m_pService->GetSection(dir->m_sKey);
		ProcessData();
		return eOSState::osContinue;
	} else if(dynamic_cast<cDummyElement*>(SelectedObject())) {
		return NavigateBack();
	} else if(cServerElement* srv = dynamic_cast<cServerElement*>(SelectedObject())) {
		m_pService = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( srv->Server() ) );
		m_pContainer = m_pService->GetSection(srv->StartPath());
		m_vServerElements.clear();
		ProcessData();
		return eOSState::osContinue;
	} else if(plexclient::Video* vid = dynamic_cast<plexclient::Video*>(SelectedObject())) {
		return eOSState::osUser1;
	} else return eOSState::osEnd;
}

eOSState cBrowserGrid::NavigateBack()
{
	std::shared_ptr<plexclient::MediaContainer> pCont = NULL;
	if(m_pService) {
		pCont = m_pService->GetLastSection();
	}

	if(pCont) {
		m_pContainer = pCont;
		ProcessData();
		return eOSState::osContinue;

	} else if(m_bServersAreRoot) {
		m_pContainer = NULL;
		m_pService = NULL;
		SetServerElements();
		ProcessData();
		return eOSState::osContinue;
	}
	return eOSState::osEnd;
}

void cBrowserGrid::DrawGrid()
{
	DrawBackground();
	m_pViewHeader->Draw(SelectedObject());
	DrawInfopane();
	DrawFooter();
}

void cBrowserGrid::DrawBackground()
{
	m_pBackground->ClearTokens();

	if(auto video = dynamic_cast<plexclient::Video*>(SelectedObject()) ) {
		bool cached = false;
		std::string path = cPictureCache::GetInstance().GetPath(video->ArtUri(), 1920, 1080, cached);
		m_pBackground->AddStringToken("selecteditembackground", path);
	}

	m_pBackground->AddIntToken("isdirectory", 1);
	m_pBackground->AddStringToken("currentdirectorybackground", "/path");
}

void cBrowserGrid::DrawInfopane()
{
	m_pInfopane->Clear();
	SelectedObject()->AddTokens(m_pInfopane, true);
}

void cBrowserGrid::DrawFooter()
{
	//if (!active)
	//   return;

	string textGreen = tr("Prev. Tab");
	string textYellow = tr("Next Tab");
	string textRed = "";
	string textBlue = "";

	if(auto vid = dynamic_cast<plexclient::Video*>(SelectedObject()) ) {
		if(vid->m_iViewCount > 0) textRed = tr("Unscrobble");
		else textRed = tr("Scrobble");
	}


	int colorKeys[4] = { Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3 };

	m_pfooter->Clear();
	m_pfooter->ClearTokens();

	m_pfooter->AddStringToken("red", textRed);
	m_pfooter->AddStringToken("green", textGreen);
	m_pfooter->AddStringToken("yellow", textYellow);
	m_pfooter->AddStringToken("blue", textBlue);

	for (int button = 1; button < 5; button++) {
		string red = *cString::sprintf("red%d", button);
		string green = *cString::sprintf("green%d", button);
		string yellow = *cString::sprintf("yellow%d", button);
		string blue = *cString::sprintf("blue%d", button);
		bool isRed = false;
		bool isGreen = false;
		bool isYellow = false;
		bool isBlue = false;
		switch (colorKeys[button-1]) {
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
		m_pfooter->AddIntToken(red, isRed);
		m_pfooter->AddIntToken(green, isGreen);
		m_pfooter->AddIntToken(yellow, isYellow);
		m_pfooter->AddIntToken(blue, isBlue);
	}

	m_pfooter->Display();
}

void cBrowserGrid::NextTab()
{
	SwitchGrid(m_pViewHeader->NextTab());
}

void cBrowserGrid::PrevTab()
{
	SwitchGrid(m_pViewHeader->PrevTab());
}

/*
 * cDummyElement
 */

void cDummyElement::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
{
	if(clear) grid->ClearTokens();
	grid->AddIntToken("isdummy", 1);
	grid->AddStringToken("title", "../");
}

std::string cDummyElement::GetTitle()
{
	return "Dummy";
}

/*
 * cServerElement
 */

cServerElement::cServerElement(plexclient::PlexServer* server, std::string startPath, std::string startName)
{
	m_pServer = server;
	m_sStartPath = startPath;
	m_sStartName = startName;
}

void cServerElement::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
{
	if(clear) grid->ClearTokens();
	grid->AddIntToken("isserver", 1);
	grid->AddStringToken("title", m_pServer->GetServerName());
	grid->AddStringToken("serverstartpointname", m_sStartName);
	grid->AddStringToken("serverip", m_pServer->GetIpAdress());
	grid->AddIntToken("serverport", m_pServer->GetPort());
	grid->AddStringToken("serverversion", m_pServer->GetVersion());
}

std::string cServerElement::GetTitle()
{
	return "Server";
}
