#ifndef PLEX_H
#define PLEX_H

#include <vdr/interface.h>
#include <vdr/plugin.h>
#include <vdr/player.h>
#include <vdr/osd.h>
#include <vdr/shutdown.h>
#include <vdr/status.h>
#include <vdr/videodir.h>
#include <vdr/menuitems.h>

#include "Config.h"
#include "Plexservice.h"

#include "plexgdm.h"
#include "cPlexOsdItem.h"
#include "hlsPlayerControl.h"

#ifdef SKINDESIGNER

#include <libskindesignerapi/skindesignerapi.h>
#include "plexSdOsd.h"
#include "pictureCache.h"
#include "SubscriptionManager.h"

#endif

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>

/// vdr-plugin version number.
/// Makefile extracts the version number for generating the file name
/// for the distribution archive.
static const char *const VERSION = "0.3.0"
#ifdef GIT_REV
"-GIT" GIT_REV
#endif
;
static const char *const DESCRIPTION = "Plex for VDR Plugin";
static const char *const MAINMENUENTRY = "Plex for VDR";


class cMyPlugin : public cPlugin {
private:
#ifdef SKINDESIGNER
    skindesignerapi::cPluginStructure *m_pPlugStruct = nullptr;
    cPlexSdOsd *m_pTestOsd = nullptr;
    static bool bSkindesigner;
    plexclient::Action action;
    bool m_bShowInfo = false;
#endif

public:
    cMyPlugin(void);

    virtual ~ cMyPlugin(void);

    virtual const char *Version(void);

    virtual const char *Description(void);

    virtual bool Initialize(void);

    virtual bool Start(void);

    virtual void MainThreadHook(void);

    virtual const char *MainMenuEntry(void);

    virtual cOsdObject *MainMenuAction(void);

    virtual cMenuSetupPage *SetupMenu(void);

    virtual bool SetupParse(const char *, const char *);

    static plexclient::cVideo CurrentVideo;
    static bool PlayingFile;

    static void PlayFile(plexclient::cVideo Vid);

public:
    static volatile bool CalledFromCode;

};

#endif
