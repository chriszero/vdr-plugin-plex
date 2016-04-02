#ifndef CVIEWHEADER_H
#define CVIEWHEADER_H

#include <memory>
#include  <libskindesignerapi/osdelements.h>
#include "viewGridNavigator.h"

enum ePlexMenuTab {
    pmtOnDeck,
    pmtRecentlyAdded,
    pmtLibrary
};

class cViewHeader {
private:
    ePlexMenuTab m_eCurrentTab;
    std::shared_ptr<skindesignerapi::cViewElement> m_pViewElem;

public:
    cViewHeader(skindesignerapi::cViewElement *viewElem);

    ~cViewHeader();

    void Draw(cGridElement *elem);

    ePlexMenuTab NextTab();

    ePlexMenuTab PrevTab();

    ePlexMenuTab CurrentTab() { return m_eCurrentTab; }
};

#endif // CVIEWHEADER_H
