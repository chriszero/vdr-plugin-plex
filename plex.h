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

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>


/// vdr-plugin version number.
/// Makefile extracts the version number for generating the file name
/// for the distribution archive.
static const char *const VERSION = "0.1.0"
#ifdef GIT_REV
    "-GIT" GIT_REV
#endif
;
static const char *const DESCRIPTION = "Plex for VDR Plugin";
static const char *const MAINMENUENTRY = "Plex for VDR";

/*
 *	Plex Browser
 */

class cPlexBrowser :public cOsdMenu
{
private:
	std::shared_ptr<plexclient::Plexservice> pService;
	std::shared_ptr<plexclient::MediaContainer> pCont;
	std::vector<plexclient::Video> *v_Vid;
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

public:
	cPlexBrowser(const char *title, std::shared_ptr<plexclient::Plexservice> Service);
	virtual eOSState ProcessKey(eKeys);

};

class cPlexInfo : public cOsdMenu
{

public:
	cPlexInfo(plexclient::Video* video);
	virtual eOSState ProcessKey(eKeys Keys);
};

/**
**	Play plugin menu class.
*/
class cPlayMenu:public cOsdMenu
{
private:
public:
	cPlayMenu(const char *, int = 0, int = 0, int = 0, int = 0, int = 0);
	virtual ~ cPlayMenu();
	virtual eOSState ProcessKey(eKeys);
};

class cMyPlugin:public cPlugin
{
public:
	cMyPlugin(void);
	virtual ~ cMyPlugin(void);
	virtual const char *Version(void);
	virtual const char *Description(void);
	virtual bool Initialize(void);
	virtual void MainThreadHook(void);
	virtual const char *MainMenuEntry(void);
	virtual cOsdObject *MainMenuAction(void);
	virtual cMenuSetupPage *SetupMenu(void);
	virtual bool SetupParse(const char *, const char *);
	
	static void PlayFile(plexclient::Video Vid);

};

#endif
