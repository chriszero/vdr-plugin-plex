#ifndef CDETAILVIEW_H
#define CDETAILVIEW_H

#include "viewGridNavigator.h"

class cDetailView
{
public:
	cDetailView(skindesignerapi::cOsdView *detailView, const cGridElement *element);
	~cDetailView();

};

#endif // CDETAILVIEW_H