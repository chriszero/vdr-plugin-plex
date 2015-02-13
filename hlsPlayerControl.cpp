#include "hlsPlayerControl.h"

#include <vdr/status.h>

#include "PlexServer.h"
#include "Plexservice.h"
#include "MediaContainer.h"
#include "PVideo.h"

// static
cControl* cHlsPlayerControl::Create(plexclient::Video Video)
{
	// Stop already playing stream
	cHlsPlayerControl* c = dynamic_cast<cHlsPlayerControl*>(cControl::Control(true));
	if(c) {
		c->Stop();
	}

	// get Metadata
	std::string uri = Video.m_Server.GetUri() + Video.m_sKey;
	plexclient::MediaContainer pmcontainer = plexclient::Plexservice::GetMediaContainer(uri);

	std::string transcodeUri =  plexclient::Plexservice::GetUniversalTranscodeUrl(&pmcontainer.m_vVideos[0], Video.m_iMyPlayOffset);
	cHlsPlayerControl* playerControl = new cHlsPlayerControl(new cHlsPlayer(transcodeUri, pmcontainer.m_vVideos[0], Video.m_iMyPlayOffset), pmcontainer.m_vVideos[0]);
	playerControl->m_title = pmcontainer.m_vVideos[0].m_sTitle;
	return playerControl;
}

cHlsPlayerControl::cHlsPlayerControl(cHlsPlayer* Player, plexclient::Video Video) :cControl(Player)
{
	dsyslog("[plex]: '%s'", __FUNCTION__);
	player = Player;
	m_Video = Video;
	//m_title = title;

	displayReplay = NULL;
	visible = modeOnly = shown = false;
	lastCurrent = lastTotal = -1;
	lastPlay = lastForward = false;
	lastSpeed = -2; // an invalid value
	timeoutShow = 0;

	cStatus::MsgReplaying(this, m_title.c_str(), m_Video.m_Media.m_sPartFile.c_str(), true);
}

cHlsPlayerControl::~cHlsPlayerControl()
{
	dsyslog("[plex]: '%s'", __FUNCTION__);
	Hide();
	delete player;
	player = NULL;
	cStatus::MsgReplaying(this, NULL, NULL, false);
}

cString cHlsPlayerControl::GetHeader(void)
{
	return m_title.c_str();
}

void cHlsPlayerControl::Hide(void)
{
	if (visible) {
		delete displayReplay;
		displayReplay = NULL;
		SetNeedsFastResponse(false);
		visible = false;
		modeOnly = false;
		lastPlay = lastForward = false;
		lastSpeed = -2; // an invalid value
		//timeSearchActive = false;
		timeoutShow = 0;
	}
}

void cHlsPlayerControl::Show(void)
{
	ShowTimed();
}

eOSState cHlsPlayerControl::ProcessKey(eKeys Key)
{
	if (!Active())
		return osEnd;
	if (Key == kPlayPause) {
		bool Play, Forward;
		int Speed;
		GetReplayMode(Play, Forward, Speed);
		if (Speed >= 0)
			Key = Play ? kPlay : kPause;
		else
			Key = Play ? kPause : kPlay;
	}
	if (visible) {
		if (timeoutShow && time(NULL) > timeoutShow) {
			Hide();
			ShowMode();
			timeoutShow = 0;
		} else if (modeOnly)
			ShowMode();
		else
			shown = ShowProgress(!shown) || shown;
	}
	bool DoShowMode = true;
	switch (int(Key)) {
		// Positioning:
	case kPlay:
	case kUp:
		Play();
		break;
	case kPause:
	case kDown:
		Pause();
		break;
	case kFastRew|k_Release:
	case kLeft|k_Release:
		//if (Setup.MultiSpeedMode) break;
	case kFastRew:
	case kLeft:
		//Backward();
		break;
	case kFastFwd|k_Release:
	case kRight|k_Release:
		//if (Setup.MultiSpeedMode) break;
	case kFastFwd:
	case kRight:
		//Forward();
		break;
	case kRed:
		//TimeSearch();
		break;
	case kGreen|k_Repeat:
	case kGreen:
		Hide();
		player->JumpRelative(-600);
		break;
	case kYellow|k_Repeat:
	case kYellow:
		Hide();
		player->JumpRelative(600);
		break;
	case kStop:
	case kBlue:
		Hide();
		Stop();
		return osEnd;
	default: {
		DoShowMode = false;
		switch (int(Key)) {
		default: {
			switch (Key) {
			case kOk:
				if (visible && !modeOnly) {
					Hide();
					DoShowMode = true;
				} else
					Show();
				break;
			case kBack:
				Hide();
				//menu = new cTitleMenu(this);
				break;
			default:
				return osUnknown;
			}
		}
		}
	}
	if (DoShowMode)
		ShowMode();
	}

	return osContinue;
}

bool cHlsPlayerControl::Active(void)
{
	return player && player->Active();
}

void cHlsPlayerControl::Pause(void)
{
	if(player)
		player->Pause();
}

void cHlsPlayerControl::Play(void)
{
	if(player)
		player->Play();
}

void cHlsPlayerControl::Stop(void)
{
	if(player)
		player->Stop();
}

void cHlsPlayerControl::SeekTo(int offset)
{
	if (player) {
		player->JumpTo(offset);
	}
}

void cHlsPlayerControl::JumpRelative(int offset)
{
	if (player)
		player->JumpRelative(offset);
}

void cHlsPlayerControl::ShowMode(void)
{
	dsyslog("[plex]: '%s'\n", __FUNCTION__);
	if (visible || Setup.ShowReplayMode && !cOsd::IsOpen()) {
		bool Play, Forward;
		int Speed;
		if (GetReplayMode(Play, Forward, Speed) && (!visible || Play != lastPlay || Forward != lastForward || Speed != lastSpeed)) {
			bool NormalPlay = (Play && Speed == -1);

			if (!visible) {
				if (NormalPlay)
					return; // no need to do indicate ">" unless there was a different mode displayed before
				visible = modeOnly = true;
				displayReplay = Skins.Current()->DisplayReplay(modeOnly);
			}

			if (modeOnly && !timeoutShow && NormalPlay)
				timeoutShow = time(NULL) + 3;
			displayReplay->SetMode(Play, Forward, Speed);
			lastPlay = Play;
			lastForward = Forward;
			lastSpeed = Speed;
		}
	}
}

bool cHlsPlayerControl::ShowProgress(bool Initial)
{
	int Current, Total;

	if (GetIndex(Current, Total)) {
		if (!visible) {
			displayReplay = Skins.Current()->DisplayReplay(modeOnly);
			//displayReplay->SetMarks(player->Marks());
			SetNeedsFastResponse(true);
			visible = true;
		}
		if (Initial) {
			lastCurrent = lastTotal = -1;
		}
		if (Current != lastCurrent || Total != lastTotal) {
			if (Setup.ShowRemainingTime || Total != lastTotal) {
				int Index = Total;
				if (Setup.ShowRemainingTime)
					Index = Current - Index;
				if(Total == 0) // Webstreams
					Index = Current;
				displayReplay->SetTotal(IndexToHMSF(Index, false, FramesPerSecond()));
				if (!Initial)
					displayReplay->Flush();
			}
			if(Total == 0) // Webstreams
				Total = Current;
			displayReplay->SetProgress(Current, Total);
			if (!Initial)
				displayReplay->Flush();
			displayReplay->SetCurrent(IndexToHMSF(Current, false, FramesPerSecond()));

			cString Title;
			//cString Pos = player ? player->PosStr() : cString(NULL);
			//if (*Pos && strlen(Pos) > 1) {
			//	Title = cString::sprintf("%s (%s)", m_title.c_str(), *Pos);
			//} else {
			Title = m_title.c_str();
			//}
			displayReplay->SetTitle(Title);

			displayReplay->Flush();
			lastCurrent = Current;
		}
		lastTotal = Total;
		ShowMode();
		return true;
	}
	return false;
}

void cHlsPlayerControl::ShowTimed(int Seconds)
{
	if (modeOnly)
		Hide();
	if (!visible) {
		shown = ShowProgress(true);
		timeoutShow = (shown && Seconds > 0) ? time(NULL) + Seconds : 0;
	} else if (timeoutShow && Seconds > 0)
		timeoutShow = time(NULL) + Seconds;
}
