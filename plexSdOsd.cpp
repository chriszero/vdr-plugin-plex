#include "plexSdOsd.h"

cPlexSdOsd::cPlexSdOsd()
{
}

void cPlexSdOsd::Show(void)
{
	bool skinDesignerAvailable = InitSkindesignerInterface("plex");
	if (!skinDesignerAvailable) {
		return;
	}

	m_pRootView = GetOsdView(eViews::viRootView);
	if (!m_pRootView) {
		esyslog("[plex]: used skindesigner skin does not support plex");
		return;
	}

	m_pViewHeader = std::shared_ptr<cViewHeader>(
	                    new cViewHeader(m_pRootView->GetViewElement(eViewElementsRoot::verHeader))
	                );
	m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
	                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser))
	                 );

	Flush();
}

void cPlexSdOsd::Flush()
{
	m_pBrowserGrid->Flush();
	
	m_pViewHeader->Draw();
	
	m_pRootView->Display();
}

eOSState cPlexSdOsd::ProcessKey(eKeys Key)
{
	switch (Key & ~k_Repeat) {
	case kUp:
		m_pBrowserGrid->NavigateUp();
		Flush();
		break;
	case kDown:
		m_pBrowserGrid->NavigateDown();
		Flush();
		break;
	case kLeft:
		m_pBrowserGrid->NavigateLeft();
		Flush();
		break;
	case kRight:
		m_pBrowserGrid->NavigateRight();
		Flush();
		break;
	case kOk:
		// Play movie or change dir
		m_pBrowserGrid->NavigateSelect();
		Flush();
		break;
	case kBack:
		m_pBrowserGrid->NavigateBack();
		Flush();
		break;
	case kRed:
		// Prev Tab
		SwitchGrid(m_pViewHeader->NextTab());
		Flush();
		break;
	case kGreen:
		// Next Tab
		SwitchGrid(m_pViewHeader->PrevTab());
		Flush();
		break;
	default:
		break;
	}
	return eOSState::osContinue;
}

void cPlexSdOsd::SwitchGrid(ePlexMenuTab currentTab)
{
	if(currentTab == ePlexMenuTab::pmtOnDeck) {
		
		auto service = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( new plexclient::PlexServer("192.168.1.175", 32400), "/hubs/home/onDeck" ) );
		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser), service)
		                 );
						 
	} else if(currentTab == ePlexMenuTab::pmtRecentlyAdded) {
		auto service = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( new plexclient::PlexServer("192.168.1.175", 32400), "/library/sections" ) );
		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser), service)
		                 );
						 
	} else if(currentTab == ePlexMenuTab::pmtLibrary) {
		
		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser))
		                 );
	}
}
