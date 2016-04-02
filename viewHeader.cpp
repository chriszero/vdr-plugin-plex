#include "viewHeader.h"
#include "tokendefinitions.h"

cViewHeader::cViewHeader(skindesignerapi::cViewElement *viewElem) {
    m_pViewElem = std::shared_ptr<skindesignerapi::cViewElement>(viewElem);
    m_eCurrentTab = ePlexMenuTab::pmtOnDeck;
}

cViewHeader::~cViewHeader() {
}

void cViewHeader::Draw(cGridElement *elem) {
    m_pViewElem->Clear();
    m_pViewElem->ClearTokens();

    elem->AddTokens(m_pViewElem, false);

    m_pViewElem->Display();
}

ePlexMenuTab cViewHeader::NextTab() {
    switch (m_eCurrentTab) {
        case ePlexMenuTab::pmtOnDeck:
            m_eCurrentTab = ePlexMenuTab::pmtRecentlyAdded;
            break;
        case ePlexMenuTab::pmtRecentlyAdded:
            m_eCurrentTab = ePlexMenuTab::pmtLibrary;
            break;
        case ePlexMenuTab::pmtLibrary:
            m_eCurrentTab = ePlexMenuTab::pmtOnDeck;
            break;
    }
    return m_eCurrentTab;
}

ePlexMenuTab cViewHeader::PrevTab() {
    switch (m_eCurrentTab) {
        case ePlexMenuTab::pmtOnDeck:
            m_eCurrentTab = ePlexMenuTab::pmtLibrary;
            break;
        case ePlexMenuTab::pmtRecentlyAdded:
            m_eCurrentTab = ePlexMenuTab::pmtOnDeck;
            break;
        case ePlexMenuTab::pmtLibrary:
            m_eCurrentTab = ePlexMenuTab::pmtRecentlyAdded;
            break;
    }
    return m_eCurrentTab;
}
