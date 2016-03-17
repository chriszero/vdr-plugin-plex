#ifndef CDISPLAYREPLAYSD_H
#define CDISPLAYREPLAYSD_H

#include "PVideo.h"
#include <memory>
#include "tokendefinitions.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cDisplayReplaySD
{
public:
	cDisplayReplaySD(plexclient::Video* video);
	~cDisplayReplaySD();

};

#endif // CDISPLAYREPLAYSD_H
