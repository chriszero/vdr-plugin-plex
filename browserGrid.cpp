#include "browserGrid.h"
#include "plexSdOsd.h"
#include "XmlObject.h"
#include "PVideo.h"
#include "Directory.h"
#include "plex.h"
#include "pictureCache.h"

cBrowserGrid::cBrowserGrid(cViewGrid* viewGrid) : cViewGridNavigator(viewGrid)
{
	m_rows = 2;
	m_columns = 5;
	m_pService = NULL;
	m_pContainer = NULL;
	m_bServersAreRoot = true;
	SetServerElements();
	ProcessData();
}

cBrowserGrid::cBrowserGrid(cViewGrid* viewGrid,  std::shared_ptr<plexclient::Plexservice> service) : cViewGridNavigator(viewGrid)
{
	m_rows = 2;
	m_columns = 5;
	m_bServersAreRoot = false;
	m_pService = service;
	m_pContainer = m_pService->GetSection(m_pService->StartUri);
	ProcessData();
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
		// Cache Images
		//m_pContainer->PreCache();
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
		cMyPlugin::PlayFile(*vid);
		return eOSState::osEnd;
	} 
	else return eOSState::osEnd;
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
		return eOSState::osBack;

	} else if(m_bServersAreRoot) {
		m_pContainer = NULL;
		m_pService = NULL;
		SetServerElements();
		ProcessData();
	}
	return eOSState::osEnd;
}

/*
 * cDummyElement
 */

void cDummyElement::AddTokens(std::shared_ptr<cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
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

void cServerElement::AddTokens(std::shared_ptr<cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
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
