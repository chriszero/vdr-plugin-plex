#ifndef CPLEXOSD_H
#define CPLEXOSD_H

#include <vdr/osdbase.h>

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>

#include "Config.h"
#include "Plexservice.h"

#include "plexgdm.h"
#include "cPlexOsdItem.h"
#include "hlsPlayerControl.h"

enum menuShow {
    MAIN,
    BROWSER
};

/*
 *	Plex Browser
 */

class cPlexBrowser : public cOsdMenu {
private:
    std::shared_ptr<plexclient::Plexservice> pService;
    std::shared_ptr<plexclient::MediaContainer> pCont;
    std::vector<plexclient::cVideo> *v_Vid;
    std::vector<plexclient::Directory> *v_Dir;
    std::vector<std::string> m_vStack;
    std::string m_sSection;
    std::string m_sActualPos;

    /// Create a browser menu for current directory
    void CreateMenu();

    /// Handle menu level up
    eOSState LevelUp(void);

    /// Handle menu item selection
    eOSState ProcessSelected();

    static std::shared_ptr<plexclient::Plexservice> pLastService;
    static int lastCurrentItem;

public:
    cPlexBrowser(const char *title, std::shared_ptr<plexclient::Plexservice> Service);

    virtual eOSState ProcessKey(eKeys);

    static cPlexBrowser *RecoverLastState();

};


/**
**	Play plugin menu class.
*/
class cPlexMenu : public cOsdMenu {
public:
    cPlexMenu(const char *, int = 0, int = 0, int = 0, int = 0, int = 0);

    virtual eOSState ProcessKey(eKeys);

    static cOsdMenu *ProcessMenu();

    static menuShow eShow;
};

#endif // CPLEXOSD_H
