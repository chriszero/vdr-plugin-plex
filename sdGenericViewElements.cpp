//
// Created by chris on 03.04.16.
//

#include "sdGenericViewElements.h"
#include "tokendefinitions.h"

cSdClock::cSdClock(std::shared_ptr<skindesignerapi::cViewElement> pViewelement) {
    m_pWatch = pViewelement;
}

bool cSdClock::DrawTime() {
    time_t t = time(0);   // get time now
    struct tm *now = localtime(&t);
    int sec = now->tm_sec;
    if (sec == m_lastsecond)
        return false;

    int min = now->tm_min;
    int hour = now->tm_hour;
    int hourMinutes = hour % 12 * 5 + min / 12;

    char monthname[20];
    char monthshort[10];
    strftime(monthshort, sizeof(monthshort), "%b", now);
    strftime(monthname, sizeof(monthname), "%B", now);

    m_pWatch->Clear();
    m_pWatch->ClearTokens();
    m_pWatch->AddIntToken((int) eTokenTimeInt::sec, sec);
    m_pWatch->AddIntToken((int) eTokenTimeInt::min, min);
    m_pWatch->AddIntToken((int) eTokenTimeInt::hour, hour);
    m_pWatch->AddIntToken((int) eTokenTimeInt::hmins, hourMinutes);
    m_pWatch->AddIntToken((int) eTokenTimeInt::year, now->tm_year + 1900);
    m_pWatch->AddIntToken((int) eTokenTimeInt::day, now->tm_mday);
    m_pWatch->AddStringToken((int) eTokenTimeStr::time, *TimeString(t));
    m_pWatch->AddStringToken((int) eTokenTimeStr::monthname, monthname);
    m_pWatch->AddStringToken((int) eTokenTimeStr::monthnameshort, monthshort);
    m_pWatch->AddStringToken((int) eTokenTimeStr::month, *cString::sprintf("%02d", now->tm_mon + 1));
    m_pWatch->AddStringToken((int) eTokenTimeStr::dayleadingzero, *cString::sprintf("%02d", now->tm_mday));
    m_pWatch->AddStringToken((int) eTokenTimeStr::dayname, *WeekDayNameFull(now->tm_wday));
    m_pWatch->AddStringToken((int) eTokenTimeStr::daynameshort, *WeekDayName(now->tm_wday));
    m_pWatch->Display();

    m_lastsecond = sec;
    return true;

}




