#include "detailView.h"
#include "Config.h"

cDetailView::cDetailView(std::shared_ptr<skindesignerapi::cOsdView> detailView, plexclient::cVideo *video)
        : cViewGridNavigator(detailView, std::shared_ptr<skindesignerapi::cViewElement>(
        detailView->GetViewElement((int) eViewElementsDetail::scrollbar))),
          cSdClock(std::shared_ptr<skindesignerapi::cViewElement>(
                  detailView->GetViewElement((int) eViewElementsDetail::watch))) {

    m_pBackground = std::shared_ptr<skindesignerapi::cViewElement>(
            detailView->GetViewElement((int) eViewElementsDetail::background));
    m_pfooter = std::shared_ptr<skindesignerapi::cViewElement>(
            detailView->GetViewElement((int) eViewElementsDetail::footer));
    m_pInfo = std::shared_ptr<skindesignerapi::cViewElement>(
            detailView->GetViewElement((int) eViewElementsDetail::info));

    m_pVideo = video;
    m_drawall = true;

    m_pGrid = NULL;
    SetViewGrid(
            std::shared_ptr<skindesignerapi::cViewGrid>(detailView->GetViewGrid((int) eViewDetailViewGrids::extras)));
    SetGridDimensions(Config::GetInstance().ExtrasGridRows, Config::GetInstance().ExtrasGridColumns);

    m_vElements.clear();

    int pos = 0;
    for (auto it = m_pVideo->m_vExtras.begin(); it != m_pVideo->m_vExtras.end(); ++it) {
        plexclient::cVideo *elem = &(*it);
        elem->AbsolutePosition = pos++;;
        m_vElements.push_back(elem);
    }

    m_startIndex = 0;

    m_setIterator = true;
    FilterElements(0);
}

void cDetailView::Flush() {
    if (m_drawall) {
        m_pBackground->Display();
        m_pInfo->Display();
        m_drawall = false;
    }
    m_pfooter->Display();
    m_pGrid->Display();
    m_pScrollbar->Display();

    m_pRootView->Display();
}

void cDetailView::Draw() {
    // Draw Grid

    DrawBackground();
    DrawFooter();
    DrawInfo();
    DrawScrollbar();
    DrawTime();
}

void cDetailView::Clear() {
    m_pBackground->Clear();
    m_pInfo->Clear();
    m_pScrollbar->Clear();
    m_pfooter->Clear();
    m_pGrid->Clear();
    m_pWatch->Clear();
}

void cDetailView::DrawBackground() {
    m_pBackground->ClearTokens();
    bool art = m_pVideo->m_sArt.empty() == false;
    m_pBackground->AddIntToken((int) eTokenDetailBackgroundInt::hasfanart, art);
    if (art) m_pBackground->AddStringToken((int) eTokenDetailBackgroundStr::fanartpath, m_pVideo->m_sArt.c_str());

    bool cover = m_pVideo->m_sThumb.empty() == false;
    m_pBackground->AddIntToken((int) eTokenDetailBackgroundInt::hascover, cover);
    if (cover) m_pBackground->AddStringToken((int) eTokenDetailBackgroundStr::coverpath, m_pVideo->m_sThumb.c_str());
}

void cDetailView::DrawFooter() {
    string textRed = tr("Play");
    string textGreen = tr("Rewind");
    string textYellow = "";
    string textBlue = "";

    if (m_pVideo->m_iViewCount > 0) textYellow = tr("Unscrobble");
    else textYellow = tr("Scrobble");

    int colorKeys[4] = {Setup.ColorKey0, Setup.ColorKey1, Setup.ColorKey2, Setup.ColorKey3};

    m_pfooter->Clear();
    m_pfooter->ClearTokens();

    m_pfooter->AddStringToken((int) eTokenFooterStr::red, textRed.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::green, textGreen.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::yellow, textYellow.c_str());
    m_pfooter->AddStringToken((int) eTokenFooterStr::blue, textBlue.c_str());

    for (int button = 0; button < 4; button++) {
        bool isRed = false;
        bool isGreen = false;
        bool isYellow = false;
        bool isBlue = false;
        switch (colorKeys[button]) {
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
        m_pfooter->AddIntToken(0 + button, isRed);
        m_pfooter->AddIntToken(4 + button, isGreen);
        m_pfooter->AddIntToken(8 + button, isYellow);
        m_pfooter->AddIntToken(12 + button, isBlue);
    }

    m_pfooter->Display();
}

void cDetailView::DrawInfo() {
    m_pInfo->Clear();
    m_pVideo->AddTokens(m_pInfo, true);
    m_pInfo->Display();
}

eOSState cDetailView::NavigateSelect() {
    if (m_setIterator) return eOSState::osContinue;

    if (dynamic_cast<plexclient::cVideo *>(SelectedObject())) {
        return eOSState::osUser1;
    } else return eOSState::osBack;
}

eOSState cDetailView::NavigateBack() {
    if (m_setIterator) return eOSState::osContinue;
    return eOSState::osBack;
}
