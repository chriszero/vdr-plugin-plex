#include "plexSdOsd.h"
#include "viewGridNavigator.h"
#include "pictureCache.h"
#include <vdr/thread.h>
#include "SubscriptionManager.h"
#include "plex.h"

cMutex cPlexSdOsd::RedrawMutex;

cPlexSdOsd::cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct, std::shared_ptr<plexclient::MediaContainer> detailContainer) : cPlexSdOsd(plugStruct) {
    m_pDetailContainer = detailContainer;
}

cPlexSdOsd::cPlexSdOsd(skindesignerapi::cPluginStructure *plugStruct) : cSkindesignerOsdObject(plugStruct) {

}

cPlexSdOsd::~cPlexSdOsd() {
    if (m_pRootView)
        m_pRootView->Deactivate(true);
    if (m_pBrowserGrid)
        m_pBrowserGrid->Clear();
    if (m_pMessage)
        m_pMessage->Clear();
}

bool cPlexSdOsd::m_bSdSupport = false;

bool cPlexSdOsd::SdSupport() {
    if (m_bSdSupport || SkindesignerAvailable()) {
        skindesignerapi::cOsdView *rootView = GetOsdView();
        if (!rootView) {
            esyslog("[plex]: used skindesigner skin does not support plex");
            return false;
        }
        delete rootView;
    }
    return (m_bSdSupport = true);
}

void cPlexSdOsd::Show(void) {
    if (!SkindesignerAvailable()) {
        return;
    }

    m_pRootView = std::shared_ptr<skindesignerapi::cOsdView>(GetOsdView((int) eViews::rootView));
    if (!m_pRootView) {
        esyslog("[plex]: used skindesigner skin does not support plex");
        return;
    }

    m_pBrowserGrid = std::shared_ptr<cBrowserGrid>(new cBrowserGrid(m_pRootView));
    m_pMessage = std::shared_ptr<skindesignerapi::cViewElement>(
            m_pRootView->GetViewElement((int) eViewElementsRoot::message));
    if(m_pDetailContainer) {
        ShowDetails(m_pDetailContainer);
    }
    Flush();
}

void cPlexSdOsd::Flush() {
    if (m_detailsActive) {
        m_pDetailGrid->Draw();
        m_pDetailGrid->Flush();
    }
    else {
        m_pBrowserGrid->DrawGrid();
        m_pBrowserGrid->Flush();
    }
}

eOSState cPlexSdOsd::ProcessKey(eKeys Key) {
    eOSState state = eOSState::osContinue;

    if (m_detailsActive) {
        state = ProcessKeyDetailView(Key);
    } else {
        //check if some plexservers are online
        if (plexclient::plexgdm::GetInstance().GetFirstServer() == NULL ||
            (plexclient::plexgdm::GetInstance().GetFirstServer() &&
             plexclient::plexgdm::GetInstance().GetFirstServer()->Offline)
                ) {
            DrawMessage(std::string(tr("No Plex Media Server found.")));
            switch (Key & ~k_Repeat) {
                case kOk:
                case kBack:
                    return eOSState::osEnd;
                default:
                    return eOSState::osContinue;
            }
        }
        state = ProcessKeyBrowserView(Key);
    }

    return state;
}


eOSState cPlexSdOsd::ProcessKeyDetailView(eKeys Key) {
    eOSState state = eOSState::osContinue;
    plexclient::cVideo *vid = NULL;

    switch (Key & ~k_Repeat) {
        case kUp:
            if (m_pDetailGrid->NavigateUp()) Flush();
            break;
        case kDown:
            if (m_pDetailGrid->NavigateDown()) Flush();
            break;
        case kLeft:
            if (m_pDetailGrid->NavigateLeft()) Flush();
            break;
        case kRight:
            if (m_pDetailGrid->NavigateRight()) Flush();
            break;
        case kOk:
            state = m_pDetailGrid->NavigateSelect();
            vid = dynamic_cast<plexclient::cVideo *>(m_pDetailGrid->SelectedObject());
            Flush();
            break;
        case kBack:
            state = eOSState::osContinue;
            m_pDetailGrid->Clear();
            m_pDetailGrid->Deactivate(true);
            m_pDetailGrid = nullptr;
            m_pDetailsView = nullptr;
            m_detailsActive = false;
            m_pBrowserGrid->Activate();
            Flush();
            break;
        case kYellow:
            if (m_pDetailGrid->GetVideo()) {
                if (m_pDetailGrid->GetVideo()->m_iViewCount > 0) m_pDetailGrid->GetVideo()->SetUnwatched();
                else m_pDetailGrid->GetVideo()->SetWatched();
                m_pDetailGrid->GetVideo()->UpdateFromServer();
                Flush();
            }
            break;
        case kGreen:
            vid = m_pDetailGrid->GetVideo();
            state = eOSState::osUser1;
            break;
        case kRed:
            vid = m_pDetailGrid->GetVideo();
            vid->m_iMyPlayOffset = vid->m_lViewoffset / 1000;
            state = eOSState::osUser1;
            break;
        case kBlue:
        default:
            break;
    }

    if (state == eOSState::osUser1 && vid) {
        cMyPlugin::PlayFile(*vid);
        state = eOSState::osEnd;
    }

    if (state != osEnd && m_pDetailGrid && m_pDetailGrid->DrawTime()) m_pDetailGrid->Flush();

    return state;
}

eOSState cPlexSdOsd::ProcessKeyBrowserView(eKeys Key) {
    eOSState state = eOSState::osContinue;
    plexclient::cVideo *vid = NULL;

    switch (Key & ~k_Repeat) {
        case kUp:
            if (m_pBrowserGrid->NavigateUp()) Flush();
            break;
        case kDown:
            if (m_pBrowserGrid->NavigateDown()) Flush();
            break;
        case kLeft:
            if (m_pBrowserGrid->NavigateLeft()) Flush();
            break;
        case kRight:
            if (m_pBrowserGrid->NavigateRight()) Flush();
            break;
        case kOk:
            // Play movie or change dir
            state = m_pBrowserGrid->NavigateSelect();
            if (state == eOSState::osUser1) {
                vid = dynamic_cast<plexclient::cVideo *>(m_pBrowserGrid->SelectedObject());
                vid->m_iMyPlayOffset = vid->m_lViewoffset / 1000;
            }
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
            vid = dynamic_cast<plexclient::cVideo *>(m_pBrowserGrid->SelectedObject());
            if (vid) {
                if (vid->m_iViewCount > 0) vid->SetUnwatched();
                else vid->SetWatched();
                vid->UpdateFromServer();
                Flush();
            }
            break;
        case kGreen: // Show Details OSD
            vid = dynamic_cast<plexclient::cVideo *>(m_pBrowserGrid->SelectedObject());
            if (vid) {
                vid->UpdateFromServer();
                ShowDetails(vid);
            }
            break;
        case kYellow:
            m_pBrowserGrid->NextTab();
            Flush();
            break;
        default:
            break;
    }

    if (state == eOSState::osUser1 && vid) {
        cMyPlugin::PlayFile(*vid);
        state = eOSState::osEnd;
    }

    if (state != osEnd && m_pBrowserGrid->DrawTime()) m_pBrowserGrid->Flush();

    return state;
}

void cPlexSdOsd::ShowDetails(plexclient::cVideo* vid) {
    m_pBrowserGrid->Deactivate(true);
    m_pDetailsView = std::shared_ptr<skindesignerapi::cOsdView>(GetOsdView((int) eViews::detailView));
    m_pDetailGrid = std::make_shared<cDetailView>(m_pDetailsView, vid);

    m_pDetailGrid->Activate();
    m_pDetailGrid->Draw();
    m_pDetailGrid->Flush();
    m_detailsActive = true;
}

void cPlexSdOsd::ShowDetails(std::shared_ptr<plexclient::MediaContainer> container) {
    if (m_detailsActive) return;

    if (container->m_vDirectories.size() > 0 || container->m_vVideos.size() > 1) {
        // show browser
        m_pBrowserGrid->ShowDirectory(container);
        m_pBrowserGrid->Activate();
        m_pBrowserGrid->Flush();
    }
    else if (container->m_vVideos.size() > 0) {
        ShowDetails(&container->m_vVideos[0]);
    }
}

void cPlexSdOsd::DrawMessage(std::string message) {
    m_pMessage->ClearTokens();
    m_pMessage->AddStringToken((int) eTokenMessageStr::message, message.c_str());
    m_pMessage->AddIntToken((int) eTokenMessageInt::displaymessage, true);
    m_pMessage->Display();
    m_pRootView->Display();
}

void cPlexSdOsd::DefineTokens(eViewElementsRoot ve, skindesignerapi::cTokenContainer *tk) {
    switch (ve) {
        case eViewElementsRoot::background:
            tk->DefineIntToken("{viewmode}", (int) eTokenBackgroundInt::viewmode);
            tk->DefineIntToken("{isdirectory}", (int) eTokenBackgroundInt::isdirectory);
            tk->DefineStringToken("{selecteditembackground}", (int) eTokenBackgroundStr::selecteditembackground);
            tk->DefineStringToken("{currentdirectorybackground}",
                                  (int) eTokenBackgroundStr::currentdirectorybackground);
            break;
        case eViewElementsRoot::infopane:
        case eViewElementsRoot::header:
            DefineGridTokens(tk);
            tk->DefineStringToken("{tabname}", (int) eTokenGridStr::tabname);
            break;
        case eViewElementsRoot::footer:
            DefineFooterTokens(tk);
            break;
        case eViewElementsRoot::watch:
            DefineWatchTokens(tk);
            break;
        case eViewElementsRoot::message:
            tk->DefineIntToken("{displaymessage}", (int) eTokenMessageInt::displaymessage);
            tk->DefineStringToken("{message}", (int) eTokenMessageStr::message);
            break;
        case eViewElementsRoot::scrollbar:
            tk->DefineIntToken("{height}", (int) eTokenScrollbarInt::height);
            tk->DefineIntToken("{offset}", (int) eTokenScrollbarInt::offset);
            tk->DefineIntToken("{hasscrollbar}", (int) eTokenScrollbarInt::hasscrollbar);
            break;
    }
}

void cPlexSdOsd::DefineGridTokens(skindesignerapi::cTokenContainer *tk) {
    tk->DefineIntToken("{viewmode}", (int) eTokenGridInt::viewmode);
    tk->DefineIntToken("{viewgroup}", (int) eTokenGridInt::viewgroup);
    tk->DefineIntToken("{viewCount}", (int) eTokenGridInt::viewCount);
    tk->DefineIntToken("{viewoffset}", (int) eTokenGridInt::viewoffset);
    tk->DefineIntToken("{viewoffsetpercent}", (int) eTokenGridInt::viewoffsetpercent);
    tk->DefineIntToken("{duration}", (int) eTokenGridInt::duration);
    tk->DefineIntToken("{year}", (int) eTokenGridInt::year);
    tk->DefineIntToken("{hasthumb}", (int) eTokenGridInt::hasthumb);
    tk->DefineIntToken("{hasart}", (int) eTokenGridInt::hasart);
    tk->DefineIntToken("{ismovie}", (int) eTokenGridInt::ismovie);
    tk->DefineIntToken("{isepisode}", (int) eTokenGridInt::isepisode);
    tk->DefineIntToken("{isdirectory}", (int) eTokenGridInt::isdirectory);
    tk->DefineIntToken("{isshow}", (int) eTokenGridInt::isshow);
    tk->DefineIntToken("{isseason}", (int) eTokenGridInt::isseason);
    tk->DefineIntToken("{isclip}", (int) eTokenGridInt::isclip);
    tk->DefineIntToken("{originallyAvailableYear}", (int) eTokenGridInt::originallyAvailableYear);
    tk->DefineIntToken("{originallyAvailableMonth}", (int) eTokenGridInt::originallyAvailableMonth);
    tk->DefineIntToken("{originallyAvailableDay}", (int) eTokenGridInt::originallyAvailableDay);
    tk->DefineIntToken("{season}", (int) eTokenGridInt::season);
    tk->DefineIntToken("{episode}", (int) eTokenGridInt::episode);
    tk->DefineIntToken("{leafCount}", (int) eTokenGridInt::leafCount);
    tk->DefineIntToken("{viewedLeafCount}", (int) eTokenGridInt::viewedLeafCount);
    tk->DefineIntToken("{childCount}", (int) eTokenGridInt::childCount);
    tk->DefineIntToken("{rating}", (int) eTokenGridInt::rating);
    tk->DefineIntToken("{hasseriesthumb}", (int) eTokenGridInt::hasseriesthumb);
    tk->DefineIntToken("{hasbanner}", (int) eTokenGridInt::hasbanner);
    tk->DefineIntToken("{columns}", (int) eTokenGridInt::columns);
    tk->DefineIntToken("{rows}", (int) eTokenGridInt::rows);
    tk->DefineIntToken("{position}", (int) eTokenGridInt::position);
    tk->DefineIntToken("{totalcount}", (int) eTokenGridInt::totalcount);
    tk->DefineIntToken("{bitrate}", (int) eTokenGridInt::bitrate);
    tk->DefineIntToken("{width}", (int) eTokenGridInt::width);
    tk->DefineIntToken("{height}", (int) eTokenGridInt::height);
    tk->DefineIntToken("{audioChannels}", (int) eTokenGridInt::audioChannels);
    tk->DefineIntToken("{isdummy}", (int) eTokenGridInt::isdummy);
    tk->DefineIntToken("{isserver}", (int) eTokenGridInt::isserver);
    tk->DefineIntToken("{serverport}", (int) eTokenGridInt::serverport);
    tk->DefineIntToken("{extratype}", (int) eTokenGridInt::extratype);

    tk->DefineStringToken("{title}", (int) eTokenGridStr::title);
    tk->DefineStringToken("{orginaltitle}", (int) eTokenGridStr::orginaltitle);
    tk->DefineStringToken("{summary}", (int) eTokenGridStr::summary);
    tk->DefineStringToken("{tagline}", (int) eTokenGridStr::tagline);
    tk->DefineStringToken("{contentrating}", (int) eTokenGridStr::contentrating);
    tk->DefineStringToken("{ratingstring}", (int) eTokenGridStr::ratingstring);
    tk->DefineStringToken("{studio}", (int) eTokenGridStr::studio);
    tk->DefineStringToken("{thumb}", (int) eTokenGridStr::thumb);
    tk->DefineStringToken("{art}", (int) eTokenGridStr::art);
    tk->DefineStringToken("{seriestitle}", (int) eTokenGridStr::seriestitle);
    tk->DefineStringToken("{seriesthumb}", (int) eTokenGridStr::seriesthumb);
    tk->DefineStringToken("{banner}", (int) eTokenGridStr::banner);
    tk->DefineStringToken("{videoResolution}", (int) eTokenGridStr::videoResolution);
    tk->DefineStringToken("{aspectRatio}", (int) eTokenGridStr::aspectRatio);
    tk->DefineStringToken("{audioCodec}", (int) eTokenGridStr::audioCodec);
    tk->DefineStringToken("{videoCodec}", (int) eTokenGridStr::videoCodec);
    tk->DefineStringToken("{container}", (int) eTokenGridStr::container);
    tk->DefineStringToken("{videoFrameRate}", (int) eTokenGridStr::videoFrameRate);
    tk->DefineStringToken("{serverstartpointname}", (int) eTokenGridStr::serverstartpointname);
    tk->DefineStringToken("{serverip}", (int) eTokenGridStr::serverip);
    tk->DefineStringToken("{serverversion}", (int) eTokenGridStr::serverversion);

    tk->DefineLoopToken("{roles[actor]}", (int) eTokenGridActorLst::roles);
    tk->DefineLoopToken("{genres[genre]}", (int) eTokenGridGenresLst::genres);
}

void cPlexSdOsd::DefineFooterTokens(skindesignerapi::cTokenContainer *tk) {
    tk->DefineIntToken("{red1}", (int) eTokenFooterInt::red1);
    tk->DefineIntToken("{red2}", (int) eTokenFooterInt::red2);
    tk->DefineIntToken("{red3}", (int) eTokenFooterInt::red3);
    tk->DefineIntToken("{red4}", (int) eTokenFooterInt::red4);
    tk->DefineIntToken("{green1}", (int) eTokenFooterInt::green1);
    tk->DefineIntToken("{green2}", (int) eTokenFooterInt::green2);
    tk->DefineIntToken("{green3}", (int) eTokenFooterInt::green3);
    tk->DefineIntToken("{green4}", (int) eTokenFooterInt::green4);
    tk->DefineIntToken("{yellow1}", (int) eTokenFooterInt::yellow1);
    tk->DefineIntToken("{yellow2}", (int) eTokenFooterInt::yellow2);
    tk->DefineIntToken("{yellow3}", (int) eTokenFooterInt::yellow3);
    tk->DefineIntToken("{yellow4}", (int) eTokenFooterInt::yellow4);
    tk->DefineIntToken("{blue1}", (int) eTokenFooterInt::blue1);
    tk->DefineIntToken("{blue2}", (int) eTokenFooterInt::blue2);
    tk->DefineIntToken("{blue3}", (int) eTokenFooterInt::blue3);
    tk->DefineIntToken("{blue4}", (int) eTokenFooterInt::blue4);
    tk->DefineStringToken("{red}", (int) eTokenFooterStr::red);
    tk->DefineStringToken("{green}", (int) eTokenFooterStr::green);
    tk->DefineStringToken("{yellow}", (int) eTokenFooterStr::yellow);
    tk->DefineStringToken("{blue}", (int) eTokenFooterStr::blue);
}

void cPlexSdOsd::DefineWatchTokens(skindesignerapi::cTokenContainer *tk) {
    tk->DefineIntToken("{sec}", (int) eTokenTimeInt::sec);
    tk->DefineIntToken("{min}", (int) eTokenTimeInt::min);
    tk->DefineIntToken("{hour}", (int) eTokenTimeInt::hour);
    tk->DefineIntToken("{hmins}", (int) eTokenTimeInt::hmins);
    tk->DefineIntToken("{day}", (int) eTokenTimeInt::day);
    tk->DefineIntToken("{year}", (int) eTokenTimeInt::year);
    tk->DefineStringToken("{time}", (int) eTokenTimeStr::time);
    tk->DefineStringToken("{dayname}", (int) eTokenTimeStr::dayname);
    tk->DefineStringToken("{daynameshort}", (int) eTokenTimeStr::daynameshort);
    tk->DefineStringToken("{dayleadingzero}", (int) eTokenTimeStr::dayleadingzero);
    tk->DefineStringToken("{month}", (int) eTokenTimeStr::month);
    tk->DefineStringToken("{monthname}", (int) eTokenTimeStr::monthname);
    tk->DefineStringToken("{monthnameshort}", (int) eTokenTimeStr::monthnameshort);
}

void cPlexSdOsd::DefineDetailsTokens(eViewElementsDetail ve, skindesignerapi::cTokenContainer *tk) {
    switch (ve) {
        case eViewElementsDetail::background:
            tk->DefineIntToken("{hasfanart}", (int) eTokenDetailBackgroundInt::hasfanart);
            tk->DefineIntToken("{hascover}", (int) eTokenDetailBackgroundInt::hascover);
            tk->DefineStringToken("{fanartpath}", (int) eTokenDetailBackgroundStr::fanartpath);
            tk->DefineStringToken("{coverpath}", (int) eTokenDetailBackgroundStr::coverpath);
            break;
        case eViewElementsDetail::footer:
            DefineFooterTokens(tk);
            break;
        case eViewElementsDetail::info:
            DefineGridTokens(tk);
            break;
        case eViewElementsDetail::message:
            tk->DefineIntToken("{displaymessage}", (int) eTokenMessageInt::displaymessage);
            tk->DefineStringToken("{message}", (int) eTokenMessageStr::message);
            break;
        case eViewElementsDetail::scrollbar:
            tk->DefineIntToken("{height}", (int) eTokenScrollbarInt::height);
            tk->DefineIntToken("{offset}", (int) eTokenScrollbarInt::offset);
            tk->DefineIntToken("{hasscrollbar}", (int) eTokenScrollbarInt::hasscrollbar);
            break;
        default:
            break;
    }
}

