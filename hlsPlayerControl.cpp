#include "hlsPlayerControl.h"

#include <vdr/status.h>
#include <vdr/remote.h>

#include <Poco/Format.h>

#include "plex.h"
#include "PlexServer.h"
#include "Plexservice.h"
#include "MediaContainer.h"
#include "PVideo.h"
#include "cPlexOsdItem.h"

// static
cControl *cHlsPlayerControl::Create(plexclient::cVideo Video) {
    // Stop already playing stream
    cHlsPlayerControl *c = dynamic_cast<cHlsPlayerControl *>(cControl::Control(true));
    if (c) {
        c->Stop();
    }

    // get Metadata
    std::string uri = Video.m_pServer->GetUri() + Video.m_sKey;
    auto pmcontainer = plexclient::Plexservice::GetMediaContainer(uri);
    if (pmcontainer == NULL) return NULL;

    std::string transcodeUri = plexclient::Plexservice::GetUniversalTranscodeUrl(&pmcontainer->m_vVideos[0],
                                                                                 Video.m_iMyPlayOffset);
    cHlsPlayerControl *playerControl = new cHlsPlayerControl(
            new cHlsPlayer(transcodeUri, pmcontainer->m_vVideos[0], Video.m_iMyPlayOffset), pmcontainer->m_vVideos[0]);
    playerControl->m_title = pmcontainer->m_vVideos[0].m_sTitle;
    return playerControl;
}

cHlsPlayerControl::cHlsPlayerControl(cHlsPlayer *Player, plexclient::cVideo Video) : cControl(Player) {
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
    menu = NULL;

    cStatus::MsgReplaying(this, m_title.c_str(), m_Video.m_Media.m_sPartFile.c_str(), true);
}

cHlsPlayerControl::~cHlsPlayerControl() {
    dsyslog("[plex]: '%s'", __FUNCTION__);
    cStatus::MsgReplaying(this, NULL, NULL, false);
    Hide();
    delete player;
    delete menu;
    player = NULL;
}

cString cHlsPlayerControl::GetHeader(void) {
    return m_title.c_str();
}

void cHlsPlayerControl::Hide(void) {
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

void cHlsPlayerControl::Show(void) {
    ShowTimed(3);
}

eOSState cHlsPlayerControl::ProcessKey(eKeys Key) {
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

    // Handle menus
    if (menu) {
        eOSState state = menu->ProcessKey(Key);
        if (state == osEnd) {
            JumpRelative(0);
            delete menu;
            menu = NULL;
        }
        if (state == osBack) {
            Hide();
            delete menu;
            menu = NULL;
        }
        return osContinue;
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
        case kFastRew:
        case kLeft:
            player->JumpRelative(-10);
            break;
        case kFastFwd:
        case kRight:
            player->JumpRelative(10);
            break;
        case kGreen | k_Repeat:
#if APIVERSNUM >= 20200
            player->JumpRelative(-Setup.SkipSecondsRepeat);
            break;
#endif
        case kGreen:
#if APIVERSNUM >= 20200
            player->JumpRelative(-Setup.SkipSeconds);
#else
            player->JumpRelative(-300);
#endif
            break;
        case kYellow | k_Repeat:
#if APIVERSNUM >= 20200
            player->JumpRelative(Setup.SkipSecondsRepeat);
            break;
#endif
        case kYellow:
#if APIVERSNUM >= 20200
            player->JumpRelative(Setup.SkipSeconds);
#else
            player->JumpRelative(300);
#endif
            break;
        case kStop:
        case kBlue:
            Hide();
            Stop();
            cMyPlugin::CalledFromCode = true;
            cRemote::CallPlugin(PLUGIN);
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
                            menu = new cStreamSelectMenu(&m_Video);
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

bool cHlsPlayerControl::Active(void) {
    return player && player->Active();
}

void cHlsPlayerControl::Pause(void) {
    if (player)
        player->Pause();
}

void cHlsPlayerControl::Play(void) {
    if (player)
        player->Play();
}

void cHlsPlayerControl::Stop(void) {
    if (player)
        player->Stop();
}

void cHlsPlayerControl::SeekTo(int offset) {
    if (player) {
        player->JumpTo(offset);
    }
}

void cHlsPlayerControl::JumpRelative(int offset) {
    if (player)
        player->JumpRelative(offset);
}

void cHlsPlayerControl::ShowMode(void) {
    //dsyslog("[plex]: '%s'\n", __FUNCTION__);
    if (visible || (Setup.ShowReplayMode && !cOsd::IsOpen())) {
        bool Play, Forward;
        int Speed;
        if (GetReplayMode(Play, Forward, Speed) &&
            (!visible || Play != lastPlay || Forward != lastForward || Speed != lastSpeed)) {
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

bool cHlsPlayerControl::ShowProgress(bool Initial) {
    int Current, Total;

    if (GetIndex(Current, Total)) {
        if (!visible) {
            displayReplay = Skins.Current()->DisplayReplay(modeOnly);
            displayReplay->SetButtons(NULL, tr("Skip Back"), tr("Skip Forward"), tr("Stop"));
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
                displayReplay->SetTotal(IndexToHMSF(Index, false, FramesPerSecond()));
                if (!Initial)
                    displayReplay->Flush();
            }
            displayReplay->SetProgress(Current, Total);
            if (!Initial)
                displayReplay->Flush();
            displayReplay->SetCurrent(IndexToHMSF(Current, false, FramesPerSecond()));
            displayReplay->SetTitle(m_title.c_str());
            displayReplay->Flush();
            lastCurrent = Current;
        }
        lastTotal = Total;
        ShowMode();
        return true;
    }
    return false;
}

void cHlsPlayerControl::ShowTimed(int Seconds) {
    if (modeOnly)
        Hide();
    if (!visible) {
        shown = ShowProgress(true);
        timeoutShow = (shown && Seconds > 0) ? time(NULL) + Seconds : 0;
    } else if (timeoutShow && Seconds > 0)
        timeoutShow = time(NULL) + Seconds;
}

cStreamSelectMenu::cStreamSelectMenu(plexclient::cVideo *Video) : cOsdMenu("StreamSelect") {
    pVideo = Video;
    CreateMenu();
}

void cStreamSelectMenu::CreateMenu() {
    SetTitle(cString::sprintf(tr("%s - Select Audio / Subtitle"), pVideo->GetTitle().c_str()));
    pVideo->UpdateFromServer();

    if (pVideo->m_Media.m_vStreams.size() > 0) {
        std::vector<plexclient::Stream> streams = pVideo->m_Media.m_vStreams;

        Add(new cOsdItem(tr("Audiostreams"), osUnknown, false));
        for (std::vector<plexclient::Stream>::iterator it = streams.begin(); it != streams.end(); ++it) {
            plexclient::Stream *pStream = &(*it);
            if (pStream->m_eStreamType == plexclient::StreamType::sAUDIO) {
                // Audio
                cString item = cString::sprintf(tr("%s%s - %s %d Channels"), pStream->m_bSelected ? "[*] " : "",
                                                pStream->m_sLanguage.c_str(), pStream->m_sCodecId.c_str(),
                                                pStream->m_iChannels);
                Add(new cPlexOsdItem(item, pStream));
            }
        }

        Add(new cOsdItem(tr("Subtitlestreams"), osUnknown, false));
        plexclient::Stream stre;
        stre.m_eStreamType = plexclient::StreamType::sSUBTITLE;
        stre.m_iID = -1;
        Add(new cPlexOsdItem(tr("None"), &stre));
        for (std::vector<plexclient::Stream>::iterator it = streams.begin(); it != streams.end(); ++it) {
            plexclient::Stream *pStream = &(*it);
            if (pStream->m_eStreamType == plexclient::StreamType::sSUBTITLE) {
                // Subtitle
                cString item = cString::sprintf("%s%s", pStream->m_bSelected ? "[*] " : "",
                                                pStream->m_sLanguage.c_str());
                Add(new cPlexOsdItem(item, pStream));
            }
        }
    }

    Display();
}

bool cStreamSelectMenu::SelectStream() {
    int current = Current();        // get current menu item index
    cPlexOsdItem *item = static_cast<cPlexOsdItem *>(Get(current));
    return pVideo->SetStream(&item->GetAttachedStream());
}

eOSState cStreamSelectMenu::ProcessKey(eKeys Keys) {
    eOSState state;

    // call standard function
    state = cOsdMenu::ProcessKey(Keys);

    switch (state) {
        case osUnknown:
            switch (Keys) {
                case kOk:
                    return SelectStream() ? osEnd : osBack;
                case kBack:
                    return osBack;
                default:
                    break;
            }
            break;
        case osBack:
            return osBack;
        default:
            break;
    }
    return state;
}
