#include "hlsPlayerControl.h"

cHlsPlayerControl::cHlsPlayerControl(cHlsPlayer* Player, std::string title) :cControl(Player)
{
	player = Player;
	m_title = title;
}

cHlsPlayerControl::~cHlsPlayerControl()
{
}

cString cHlsPlayerControl::GetHeader(void)
{
	return m_title.c_str();
}

void cHlsPlayerControl::Hide(void)
{
}

void cHlsPlayerControl::Show(void)
{
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
		//SkipSeconds(-60);
		break;
	case kYellow|k_Repeat:
	case kYellow:
		//SkipSeconds( 60);
		break;
	case kStop:
	case kBlue:
		Hide();
		Stop();
		return osEnd;
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
