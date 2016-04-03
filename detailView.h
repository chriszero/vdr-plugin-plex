#ifndef CDETAILVIEW_H
#define CDETAILVIEW_H

#include "viewGridNavigator.h"
#include "PVideo.h"

#include "tokendefinitions.h"
#include "sdGenericViewElements.h"
#include  <libskindesignerapi/osdelements.h>
#include  <libskindesignerapi/skindesignerosdbase.h>

class cDetailView : public cViewGridNavigator, public cSdClock {
public:
    cDetailView(std::shared_ptr<skindesignerapi::cOsdView> detailView, plexclient::cVideo *video);

    void Draw();

    virtual void Flush();

    virtual eOSState NavigateSelect();

    virtual eOSState NavigateBack();

    plexclient::cVideo *GetVideo() { return m_pVideo; };

    virtual void Clear();

private:
    std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
    std::shared_ptr<skindesignerapi::cViewElement> m_pfooter;
    std::shared_ptr<skindesignerapi::cViewElement> m_pInfo;

    plexclient::cVideo *m_pVideo;
    bool m_drawall;

    void DrawBackground();

    void DrawFooter();

    void DrawInfo();
};

#endif // CDETAILVIEW_H
