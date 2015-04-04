#include "plexSdOsd.h"
#include "viewGridNavigator.h"
#include "pictureCache.h"
#include <vdr/thread.h>

cMutex cPlexSdOsd::RedrawMutex;

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
	m_pBackground = std::shared_ptr<cViewElement>(m_pRootView->GetViewElement(eViewElementsRoot::verBackground));

	m_pViewHeader = std::shared_ptr<cViewHeader>(
	                    new cViewHeader(m_pRootView->GetViewElement(eViewElementsRoot::verHeader))
	                );
	m_pfooter = std::shared_ptr<cViewElement>(m_pRootView->GetViewElement(eViewElementsRoot::verFooter));

	SwitchGrid(m_pViewHeader->CurrentTab());

	Flush();
}

void cPlexSdOsd::Flush()
{
	cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
	DrawBackground();
	m_pViewHeader->Draw(m_pBrowserGrid->SelectedObject());

	m_pBackground->Display();

	m_pBrowserGrid->Flush();

	m_pRootView->Display();
}

void cPlexSdOsd::DrawBackground()
{
	m_pBackground->ClearTokens();

	if(auto video = dynamic_cast<plexclient::Video*>(m_pBrowserGrid->SelectedObject()) ) {
		bool cached = false;
		std::string path = cPictureCache::GetInstance().GetPath(video->ArtUri(), 1920, 1080, cached);
		m_pBackground->AddStringToken("selecteditembackground", path);
	}

	m_pBackground->AddIntToken("isdirectory", 1);
	m_pBackground->AddStringToken("currentdirectorybackground", "/path");
}

eOSState cPlexSdOsd::ProcessKey(eKeys Key)
{
	eOSState state = eOSState::osContinue;
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
		state = m_pBrowserGrid->NavigateSelect();
		Flush();
		break;
	case kBack:
		//state =
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
	return state;
}

void cPlexSdOsd::SwitchGrid(ePlexMenuTab currentTab)
{
	if(currentTab == ePlexMenuTab::pmtOnDeck) {
		auto service = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( plexclient::plexgdm::GetInstance().GetServer("192.168.1.175", 32400), "/hubs/home/onDeck" ) );
		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser), service)
		                 );
		m_pBrowserGrid->m_pRootView = m_pRootView;

	} else if(currentTab == ePlexMenuTab::pmtRecentlyAdded) {
		auto service = std::shared_ptr<plexclient::Plexservice>(new plexclient::Plexservice( plexclient::plexgdm::GetInstance().GetServer("192.168.1.175", 32400), "/library/sections" ) );
		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser), service)
		                 );
		m_pBrowserGrid->m_pRootView = m_pRootView;

	} else if(currentTab == ePlexMenuTab::pmtLibrary) {

		m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(
		                     new cBrowserGrid( m_pRootView->GetViewGrid(eViewGrids::vgBrowser))
		                 );
		m_pBrowserGrid->m_pRootView = m_pRootView;
	}
}

void cPlexSdOsd::DrawFooter()
{
	//if (!active)
	//   return;

	string textGreen = tr("Prev. Tab");
	string textYellow = tr("Next Tab");
	string textRed = "";
	string textBlue = "";

	if(auto vid = dynamic_cast<plexclient::Video*>(m_pBrowserGrid->SelectedObject()) ) {
		if(vid->m_iViewCount > 0) textRed = tr("Unscrobble");
		else textRed = tr("Scrobble");
		textBlue = tr("Info");
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
