#include "plexSdOsd.h"
#include "viewGridNavigator.h"
#include "pictureCache.h"
#include <vdr/thread.h>
#include "SubscriptionManager.h"
#include "plex.h"

cMutex cPlexSdOsd::RedrawMutex;

cPlexSdOsd::cPlexSdOsd()
{
}

cPlexSdOsd::~cPlexSdOsd()
{
	cPictureCache::GetInstance().RemoveAll();
}

bool cPlexSdOsd::SdSupport()
{
	bool skinDesignerAvailable = InitSkindesignerInterface("plex");
	if (skinDesignerAvailable) {

		skindesignerapi::cOsdView *rootView = GetOsdView(eViews::viRootView);
		if (!rootView) {
			esyslog("[plex]: used skindesigner skin does not support plex");
			return false;
		}
	}
	return skinDesignerAvailable;
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

	m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(new cBrowserGrid(m_pRootView));
	Flush();
}

void cPlexSdOsd::Flush()
{
	m_pBrowserGrid->DrawGrid();
	m_pBrowserGrid->Flush();
}

eOSState cPlexSdOsd::ProcessKey(eKeys Key)
{
	eOSState state = eOSState::osContinue;
	plexclient::Video* vid = dynamic_cast<plexclient::Video*>(m_pBrowserGrid->SelectedObject());
	
	if (m_pBrowserGrid->DrawTime())
            m_pBrowserGrid->Flush();
	
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
		if(vid) {
			if(vid->m_iViewCount > 0) vid->SetUnwatched();
			else vid->SetWatched();
			vid->UpdateFromServer();
			Flush();
		}
		break;
	case kGreen:
		m_pBrowserGrid->PrevTab();
		Flush();
		break;
	case kYellow:
		m_pBrowserGrid->NextTab();
		Flush();
		break;
	default:
		break;
	}

	if(state == eOSState::osUser1) {
		//plexclient::ActionManager::GetInstance().AddAction(*vid);
		cMyPlugin::PlayFile(*vid);
		state = eOSState::osEnd;
	}
	return state;
}
