#include "browserGrid.h"
#include "plexSdOsd.h"
#include "XmlObject.h"
#include "PVideo.h"
#include "Directory.h"
#include "plex.h"
#include "pictureCache.h"

cBrowserGrid::cBrowserGrid(std::shared_ptr<skindesignerapi::cOsdView> rootView) : cViewGridNavigator(rootView)
{
	m_pBackground = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verBackground));
	m_pHeader = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verHeader));
	m_pfooter = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verFooter));
	m_pInfopane = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verInfopane));
	m_pWatch = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verWatch));
	m_pScrollbar = std::shared_ptr<skindesignerapi::cViewElement>(rootView->GetViewElement(eViewElementsRoot::verScrollbar));
	m_lastsecond = 0;

	m_pService = NULL;
	m_pContainer = NULL;
	m_viewEntryIndex = 0;
	m_redrawBackground = true;

	Config *conf = &Config::GetInstance();
	if(conf->DefaultViewMode == ViewMode::Cover) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid(eViewGrids::vgCover) ));
		SetGridDimensions(conf->CoverGridRows, conf->CoverGridColumns);
	} else if(conf->DefaultViewMode == ViewMode::Detail) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid(eViewGrids::vgDetail) ));
		SetGridDimensions(conf->DetailGridRows, conf->DetailGridColumns);
	} else if(conf->DefaultViewMode == ViewMode::List) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(rootView->GetViewGrid(eViewGrids::vgList) ));
		SetGridDimensions(conf->ListGridRows, conf->ListGridColumns);
	}

	SwitchGrid(m_viewEntryIndex);
	SwitchView();
}

cBrowserGrid::~cBrowserGrid()
{
	m_vServerElements.clear();
	m_vElements.clear();
}

void cBrowserGrid::Clear()
{
	m_pBackground->Clear();
	m_pfooter->Clear();
	m_pInfopane->Clear();
	m_pWatch->Clear();
	m_pGrid->Clear();
}

void cBrowserGrid::Flush()
{
	if(m_redrawBackground){
		m_pBackground->Display();
		m_redrawBackground = false;
	}
	
	cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
	m_pGrid->Display();
	m_pScrollbar->Display();

	m_pRootView->Display();
}

void cBrowserGrid::SwitchView(ViewMode mode)
{
	auto selObj = SelectedObject();
	if(!selObj) return;

	Config *conf = &Config::GetInstance();
	conf->DefaultViewMode = mode;
	if(conf->DefaultViewMode == ViewMode::Cover) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid(eViewGrids::vgCover) ));
		SetGridDimensions(conf->CoverGridRows, conf->CoverGridColumns);
	} else if(conf->DefaultViewMode == ViewMode::Detail) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid(eViewGrids::vgDetail) ));
		SetGridDimensions(conf->DetailGridRows, conf->DetailGridColumns);
	} else if(conf->DefaultViewMode == ViewMode::List) {
		SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid>(m_pRootView->GetViewGrid(eViewGrids::vgList) ));
		SetGridDimensions(conf->ListGridRows, conf->ListGridColumns);
	}

	int activePos = selObj->AbsolutePosition;
	//ProcessData();

	for(std::vector<cGridElement*>::iterator it = m_vElements.begin(); it != m_vElements.end(); ++it) {
		cGridElement *elem = *it;
		elem->Position = -1;
		elem->Dirty();
		elem->SetPosition(-1,-1);
	}

	m_pGrid->Clear();
	m_startIndex = activePos;
	m_setIterator = true;
	FilterElements(0);
}

void cBrowserGrid::NextViewMode()
{
	ViewMode mode = Config::GetInstance().DefaultViewMode;
	if(mode == ViewMode::Cover) {
		mode = ViewMode::Detail;
	} else if(mode == ViewMode::Detail) {
		mode = ViewMode::List;
	} else if(mode == ViewMode::List) {
		mode = ViewMode::Cover;
	}
	SwitchView(mode);
}

void cBrowserGrid::SwitchGrid(int index)
{
	cPictureCache::GetInstance().RemoveAll();

	m_pHeader->Clear();
	m_pHeader->ClearTokens();
	m_pHeader->AddIntToken("columns", m_columns);
	m_pHeader->AddIntToken("rows", m_rows);
	m_pHeader->AddIntToken("totalcount", m_vElements.size());

	if(plexclient::plexgdm::GetInstance().GetFirstServer()) {
		if(m_viewEntryIndex < Config::GetInstance().m_viewentries.size()) {
			ViewEntry entry = Config::GetInstance().m_viewentries[index];
			m_pHeader->AddStringToken("tabname", tr(entry.Name.c_str()));
			m_pService = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( plexclient::plexgdm::GetInstance().GetFirstServer(), entry.PlexPath ) );
			m_pContainer = m_pService->GetSection(m_pService->StartUri);
			m_bServersAreRoot = false;
			m_vServerElements.clear();
		} else {
			//Server View
			m_pHeader->AddStringToken("tabname", tr("Library"));
			m_pService = NULL;
			m_pContainer = NULL;
			m_bServersAreRoot = true;
			SetServerElements();
		}
	} else {
		m_pHeader->AddStringToken("tabname", tr("No Plex Media Server found."));
		m_pInfopane->AddStringToken("title", tr("No Plex Media Server found."));
		m_pService = NULL;
		m_pContainer = NULL;
	}
	ProcessData();
	auto selObj = SelectedObject();
	if(selObj) {
		selObj->AddTokens(m_pHeader, false);
		m_pHeader->AddIntToken("position", selObj->AbsolutePosition);
	}
	
	DrawBackground();
}

void cBrowserGrid::SetServerElements()
{
	m_vServerElements.clear();

	for(std::vector<plexclient::PlexServer>::iterator it = plexclient::plexgdm::GetInstance().GetPlexservers().begin(); it != plexclient::plexgdm::GetInstance().GetPlexservers().end(); ++it) {
		for(std::vector<ViewEntry>::iterator vEntry = Config::GetInstance().m_serverViewentries.begin(); vEntry != Config::GetInstance().m_serverViewentries.end(); ++vEntry) {
			m_vServerElements.push_back(cServerElement(&(*it), vEntry->PlexPath, vEntry->Name));
		}
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

	int pos = 0;
	for(std::vector<cGridElement*>::iterator it = m_vElements.begin(); it != m_vElements.end(); ++it) {
		cGridElement *elem = *it;
		elem->AbsolutePosition = pos++;
	}

	m_startIndex = 0;

	m_pGrid->Clear();
	m_setIterator = true;
	FilterElements(0);
}

eOSState cBrowserGrid::NavigateSelect()
{
	if(m_setIterator) return eOSState::osContinue;;
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
	if(m_setIterator) return eOSState::osContinue;;
	std::shared_ptr<plexclient::MediaContainer> pCont = NULL;
	if(m_pService) {
		pCont = m_pService->GetLastSection();
	}

	if(pCont) {
		m_pContainer = pCont;
		ProcessData();
		return eOSState::osContinue;

	} else if(m_bServersAreRoot && m_pService) {
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
	m_pHeader->Display();
	DrawInfopane();
	DrawFooter();
	DrawScrollbar();
}

void cBrowserGrid::DrawBackground()
{
	m_redrawBackground = true;
	m_pBackground->ClearTokens();

	/*auto video = dynamic_cast<plexclient::Video*>(SelectedObject());
	if(video) {
		bool cached = false;
		std::string path = cPictureCache::GetInstance().GetPath(video->ArtUri(), 1920, 1080, cached);
		m_pBackground->AddStringToken("selecteditembackground", path);
	}
	*/
	m_pBackground->AddIntToken("isdirectory", 1);
	m_pBackground->AddStringToken("currentdirectorybackground", "/path");
	m_pBackground->AddIntToken("viewmode", Config::GetInstance().DefaultViewMode);
}

void cBrowserGrid::DrawInfopane()
{
	m_pInfopane->Clear();
	if(SelectedObject()) {
		SelectedObject()->AddTokens(m_pInfopane, true);
		m_pInfopane->AddIntToken("columns", m_columns);
		m_pInfopane->AddIntToken("rows", m_rows);
		m_pInfopane->AddIntToken("totalcount", m_vElements.size());
		m_pInfopane->AddIntToken("position", SelectedObject()->AbsolutePosition);
	}
	m_pInfopane->Display();
}

void cBrowserGrid::DrawFooter()
{
	//if (!active)
	//   return;
	cString nextTab = "Library";
	if(m_viewEntryIndex + 1 < Config::GetInstance().m_viewentries.size()) {
		nextTab = Config::GetInstance().m_viewentries[m_viewEntryIndex + 1].Name.c_str();
	} else if(m_viewEntryIndex + 1 == Config::GetInstance().m_viewentries.size() + 1) {
		nextTab = Config::GetInstance().m_viewentries[0].Name.c_str();
	}
	cString prevTab = "Library";
	if(m_viewEntryIndex - 1 >= 0) {
		prevTab = Config::GetInstance().m_viewentries[m_viewEntryIndex - 1].Name.c_str();
	}

	string textGreen = tr(prevTab);
	string textYellow = tr(nextTab);
	string textRed = "";
	string textBlue = tr("Switch View");

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

void cBrowserGrid::DrawScrollbar()
{
	
	if (m_vElements.size() == 0)
		return;
	
	int currentRow = SelectedObject()->AbsolutePosition / m_columns;
	int totalRows = m_vElements.size() / m_columns;
	
	int scrollBarHeight = 100.0 / totalRows * m_rows;  

	int offset = 100.0 / totalRows * currentRow;
	if(offset >= 100 - scrollBarHeight) {
		offset = 100.0 - scrollBarHeight;
	}
	m_pScrollbar->Clear();
	m_pScrollbar->ClearTokens();
	
	m_pScrollbar->AddIntToken("height", scrollBarHeight);
	m_pScrollbar->AddIntToken("offset", offset);
	m_pScrollbar->Display();
}

void cBrowserGrid::NextTab()
{
	m_viewEntryIndex++;
	if(m_viewEntryIndex > Config::GetInstance().m_viewentries.size()) {
		m_viewEntryIndex = 0;
	}
	SwitchGrid(m_viewEntryIndex);
}

void cBrowserGrid::PrevTab()
{
	m_viewEntryIndex--;
	if(m_viewEntryIndex < 0) {
		m_viewEntryIndex = Config::GetInstance().m_viewentries.size();
	}
	SwitchGrid(m_viewEntryIndex);
}

bool cBrowserGrid::DrawTime()
{
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	int sec = now->tm_sec;
	if (sec == m_lastsecond)
		return false;

	int min = now->tm_min;
	int hour = now->tm_hour;
	int hourMinutes = hour%12 * 5 + min / 12;

	char monthname[20];
	char monthshort[10];
	strftime(monthshort, sizeof(monthshort), "%b", now);
	strftime(monthname, sizeof(monthname), "%B", now);

	m_pWatch->Clear();
	m_pWatch->ClearTokens();
	m_pWatch->AddIntToken("sec", sec);
	m_pWatch->AddIntToken("min", min);
	m_pWatch->AddIntToken("hour", hour);
	m_pWatch->AddIntToken("hmins", hourMinutes);
	m_pWatch->AddIntToken("year", now->tm_year + 1900);
	m_pWatch->AddIntToken("day", now->tm_mday);
	m_pWatch->AddStringToken("time", *TimeString(t));
	m_pWatch->AddStringToken("monthname", monthname);
	m_pWatch->AddStringToken("monthnameshort", monthshort);
	m_pWatch->AddStringToken("month", *cString::sprintf("%02d", now->tm_mon + 1));
	m_pWatch->AddStringToken("dayleadingzero", *cString::sprintf("%02d", now->tm_mday));
	m_pWatch->AddStringToken("dayname", *WeekDayNameFull(now->tm_wday));
	m_pWatch->AddStringToken("daynameshort", *WeekDayName(now->tm_wday));
	m_pWatch->Display();

	m_lastsecond = sec;
	return true;
}

/*
 * cDummyElement
 */

cDummyElement::cDummyElement()
{
	m_title = "../";
}

cDummyElement::cDummyElement(std::string title)
{
	m_title = title;
}

void cDummyElement::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
{
	if(clear) grid->ClearTokens();
	grid->AddIntToken("isdummy", 1);
	grid->AddStringToken("title", m_title);
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
	grid->AddIntToken("viewmode", Config::GetInstance().DefaultViewMode);
}

std::string cServerElement::GetTitle()
{
	return "Server";
}
