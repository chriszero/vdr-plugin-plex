#include "sdDisplayReplay.h"
#include "plexSdOsd.h"

cSdDisplayReplay::cSdDisplayReplay(plexclient::Video Video, cHlsPlayerControl* Control)
{
	m_video = Video;
	m_pContol = Control;
	m_pRootView = NULL;
	m_lastsecond = 0;
}

cSdDisplayReplay::~cSdDisplayReplay()
{
	delete m_pRootView;
}

void cSdDisplayReplay::Show(void)
{
	bool skinDesignerAvailable = InitSkindesignerInterface("plex");
	if (!skinDesignerAvailable) {
		return;
	}

	m_pRootView = GetOsdView(eViews::viReplay);
	if (!m_pRootView) {
		esyslog("[plex]: used skindesigner skin does not support plex");
		return;
	}
	m_pInfo = std::shared_ptr<skindesignerapi::cViewElement>(m_pRootView->GetViewElement(eElements::Info));
	m_pReplay = std::shared_ptr<skindesignerapi::cViewElement>(m_pRootView->GetViewElement(eElements::Replay));
	m_pTime =  std::shared_ptr<skindesignerapi::cViewElement>(m_pRootView->GetViewElement(eElements::Time));
	m_pBackground = std::shared_ptr<skindesignerapi::cViewElement>(m_pRootView->GetViewElement(eElements::Background));
}

//eOSState cSdDisplayReplay::ProcessKey(eKeys Key)
//{
//}

void cSdDisplayReplay::DrawInfo()
{
	m_pInfo->Clear();
	m_video.AddTokens(m_pInfo);
}

void cSdDisplayReplay::DrawReplay()
{
	m_pReplay->ClearTokens();
	m_video.AddTokens(m_pReplay, false);

	int current, total;
	m_pContol->GetIndex(current, total);

	m_pReplay->AddIntToken("progresstotal", total / m_pContol->FramesPerSecond());
	m_pReplay->AddIntToken("progresscurrent", current / m_pContol->FramesPerSecond());

	bool play, forward;
	int speed;
	m_pContol->GetReplayMode(play, forward, speed);

	m_pReplay->AddIntToken("playing", speed >= 0);
	m_pReplay->AddIntToken("pausing", !play);
	//m_pReplay->AddIntToken("seeking", 0);
	m_pReplay->AddStringToken("totaltime", IndexToHMS(total));
	m_pReplay->AddStringToken("currenttime", IndexToHMS(current));
	m_pReplay->AddStringToken("endtime", "hh:mm");
}

void cSdDisplayReplay::Flush()
{
	m_pInfo->Display();
	m_pReplay->Display();
	m_pRootView->Display();
}

bool cSdDisplayReplay::DrawTime()
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

	m_pTime->Clear();
	m_pTime->ClearTokens();
	m_pTime->AddIntToken("sec", sec);
	m_pTime->AddIntToken("min", min);
	m_pTime->AddIntToken("hour", hour);
	m_pTime->AddIntToken("hmins", hourMinutes);
	m_pTime->AddIntToken("year", now->tm_year + 1900);
	m_pTime->AddIntToken("day", now->tm_mday);
	m_pTime->AddStringToken("time", *TimeString(t));
	m_pTime->AddStringToken("monthname", monthname);
	m_pTime->AddStringToken("monthnameshort", monthshort);
	m_pTime->AddStringToken("month", *cString::sprintf("%02d", now->tm_mon + 1));
	m_pTime->AddStringToken("dayleadingzero", *cString::sprintf("%02d", now->tm_mday));
	m_pTime->AddStringToken("dayname", *WeekDayNameFull(now->tm_wday));
	m_pTime->AddStringToken("daynameshort", *WeekDayName(now->tm_wday));
	m_pTime->Display();

	m_lastsecond = sec;
	return true;
}

std::string cSdDisplayReplay::IndexToHMS(int Index, bool WithSeconds)
{
	std::stringstream ss;
	if (Index < 0) {
		Index = -Index;
		ss << "-";
	}
	
	int s = Index / m_pContol->FramesPerSecond();
	int m = s / 60 % 60;
	int h = s / 3600;
	s %= 60;
	ss << h << ":" << m;
	
	if(WithSeconds) {
		ss << ":" << s;
	}
	return ss.str();
}
