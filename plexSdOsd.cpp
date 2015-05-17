#include "plexSdOsd.h"
#include "viewGridNavigator.h"
#include "pictureCache.h"
#include <vdr/thread.h>
#include "SubscriptionManager.h"
#include "plex.h"

cMutex cPlexSdOsd::RedrawMutex;

cPlexSdOsd::cPlexSdOsd()
{
	m_pRootView = NULL;
}

cPlexSdOsd::~cPlexSdOsd()
{
	if(m_pBrowserGrid)
		m_pBrowserGrid->Clear();
	if(m_pMessage)
		m_pMessage->Clear();
	cPictureCache::GetInstance().RemoveAll();
	delete m_pRootView;
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
	m_pMessage = std::shared_ptr<skindesignerapi::cViewElement>(m_pRootView->GetViewElement(eViewElementsRoot::verMessage));
	m_messageDisplayed = false;
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

	if(m_messageDisplayed) {
		switch (Key & ~k_Repeat) {
		case kOk:
			vid->m_iMyPlayOffset = vid->m_lViewoffset/1000;
			m_messageDisplayed = false;
			state = eOSState::osUser1;
			break;
		case kBack:
			vid->m_lViewoffset = 0;
			state = eOSState::osUser1;
		default:
			break;
		}
	} else {

		switch (Key & ~k_Repeat) {
		case kUp:
			if(m_pBrowserGrid->NavigateUp()) Flush();
			break;
		case kDown:
			if(m_pBrowserGrid->NavigateDown()) Flush();
			break;
		case kLeft:
			if(m_pBrowserGrid->NavigateLeft()) Flush();
			break;
		case kRight:
			if(m_pBrowserGrid->NavigateRight()) Flush();
			break;
		case kOk:
			// Play movie or change dir
			state = m_pBrowserGrid->NavigateSelect();
			Flush();
			break;
		case kBack:
			state = m_pBrowserGrid->NavigateBack();
			Flush();
			break;
		case kBlue:
			m_pBrowserGrid->NextViewMode();
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

	}

	if(state == eOSState::osUser1) {
		if(vid->m_iMyPlayOffset == 0 && vid->m_lViewoffset > 0 ) {
			cString message = cString::sprintf(tr("'Ok' to start from %ld minutes, 'Back' to start from beginning."), vid->m_lViewoffset / 60000);
			DrawMessage(std::string(message));
			m_messageDisplayed = true;
		} else {
			cMyPlugin::PlayFile(*vid);
			state = eOSState::osEnd;
		}
	}
	return state;
}

void cPlexSdOsd::DrawMessage(std::string message)
{
	m_pMessage->ClearTokens();
	m_pMessage->AddStringToken("message", message);
	m_pMessage->AddIntToken("displaymessage", true);
	m_pMessage->Display();
	m_pRootView->Display();
}
