#ifndef CDISPLAYREPLAYSD_H
#define CDISPLAYREPLAYSD_H

#include "PVideo.h"
#include <memory>
#include "tokendefinitions.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cDisplayReplaySD : public skindesignerapi::cSkindesignerOsdObject {
private:
    static skindesignerapi::cPluginStructure *m_pPlugStructReplay;

    static skindesignerapi::cPluginStructure *GetPluginStruct();

    std::shared_ptr<skindesignerapi::cOsdView> m_pRootView;
    std::shared_ptr<skindesignerapi::cViewElement> m_pProgessbar;
    std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
    std::shared_ptr<skindesignerapi::cViewElement> m_pVideoinfo;
    std::shared_ptr<skindesignerapi::cViewElement> m_pTranscodeinfo;

public:
    cDisplayReplaySD(plexclient::cVideo *video);

    ~cDisplayReplaySD();

    virtual void Show(void);

    virtual eOSState ProcessKey(eKeys Key);

    void Flush();

    void SetCurrent(const char *Current);

    void SetMode(bool Play, bool Forward, int Speed);


};

#endif // CDISPLAYREPLAYSD_H
