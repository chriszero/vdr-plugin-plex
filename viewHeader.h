#ifndef CVIEWHEADER_H
#define CVIEWHEADER_H

#include <memory>
#include "libskindesigner/osdelements.h"

enum ePlexMenuTab {
	pmtOnDeck,
	pmtRecentlyAdded,
	pmtLibrary
};

class cViewHeader
{
private:
	ePlexMenuTab m_eCurrentTab;
	std::shared_ptr<cViewElement> m_pViewElem;
	
public:
	cViewHeader(cViewElement* viewElem);
	void Draw();
	ePlexMenuTab NextTab();
	ePlexMenuTab PrevTab();
	ePlexMenuTab CurrentTab() { return m_eCurrentTab; }
};

#endif // CVIEWHEADER_H
