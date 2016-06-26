#ifndef CBROWSERGRID_H
#define CBROWSERGRID_H

#include <memory>
#include <vector>
#include <functional>
#include "Plexservice.h"
#include "plexgdm.h"
#include "PlexServer.h"
#include "viewGridNavigator.h"
#include "sdGenericViewElements.h"
//#include "viewHeader.h"
#include <libskindesignerapi/osdelements.h>

class cDummyElement : public cGridElement {
private:
    std::string m_title;

public:
    cDummyElement(std::string title);

    cDummyElement();

    virtual std::string GetTitle();

    virtual void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear = true,
                           std::function<void(cGridElement *)> OnCached = NULL);
};

class cServerElement : public cGridElement {
private:
    plexclient::PlexServer *m_pServer;
    std::string m_sStartPath;
    std::string m_sStartName;
public:
    cServerElement(plexclient::PlexServer *server, std::string startPath, std::string startName);

    virtual std::string GetTitle();

    virtual void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear = true,
                           std::function<void(cGridElement *)> OnCached = NULL);

    std::string StartPath() { return m_sStartPath; }

    plexclient::PlexServer *Server() { return m_pServer; }
};

class cBrowserGrid : public cViewGridNavigator, public cSdClock {
private:
    //std::shared_ptr<cViewHeader> m_pViewHeader;
    std::shared_ptr<skindesignerapi::cViewElement> m_pHeader;
    std::shared_ptr<skindesignerapi::cViewElement> m_pBackground;
    std::shared_ptr<skindesignerapi::cViewElement> m_pfooter;
    std::shared_ptr<skindesignerapi::cViewElement> m_pInfopane;

    int m_viewEntryIndex;
    bool m_redrawBackground;

    bool m_bServersAreRoot;
    std::vector<cServerElement> m_vServerElements;
    std::shared_ptr<plexclient::MediaContainer> m_pContainer;
    std::shared_ptr<plexclient::Plexservice> m_pService;
    cDummyElement m_Dummy;

    void ProcessData();

    void SetServerElements();

    void DrawFooter();

    void DrawBackground();

    void DrawInfopane();

public:
    cBrowserGrid(std::shared_ptr<skindesignerapi::cOsdView> rootView);

    ~cBrowserGrid();

    //cBrowserGrid(skindesignerapi::cViewGrid* viewGrid, std::shared_ptr<plexclient::Plexservice> service);
    std::shared_ptr<plexclient::MediaContainer> MediaContainer() { return m_pContainer; }

    void DrawGrid();

    void SwitchGrid(int index);

    void SwitchView(ViewMode mode = Config::GetInstance().DefaultViewMode);

    void NextViewMode();

    void ShowDirectory(std::shared_ptr<plexclient::MediaContainer> container);

    virtual eOSState NavigateSelect();

    virtual eOSState NavigateBack();

    void NextTab();

    void PrevTab();

    virtual void Flush();

    virtual void Clear();
};

#endif // CBROWSERGRID_H
