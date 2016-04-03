//
// Created by chris on 03.04.16.
//

#ifndef VDR_PLUGIN_PLEX_SDGENERICVIEWELEMENTS_H
#define VDR_PLUGIN_PLEX_SDGENERICVIEWELEMENTS_H

#include <memory>
#include <libskindesignerapi/osdelements.h>

class cSdClock {
private:
    int m_lastsecond = 0;

protected:
    std::shared_ptr<skindesignerapi::cViewElement> m_pWatch = nullptr;

public:
    cSdClock(std::shared_ptr<skindesignerapi::cViewElement> pViewelement);

    bool DrawTime();
};

#endif //VDR_PLUGIN_PLEX_SDGENERICVIEWELEMENTS_H
