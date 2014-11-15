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
#include "play_service.h"
#include "cPlexOsdItem.h"

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>


/**
**	Device plugin remote class.
*/
class cMyRemote:public cRemote
{
  public:

    /**
    **	Soft device remote class constructor.
    **
    **	@param name	remote name
    */
    cMyRemote(const char *name):cRemote(name)
    {
    }

    /**
    **	Put keycode into vdr event queue.
    **
    **	@param code	key code
    **	@param repeat	flag key repeated
    **	@param release	flag key released
    */
    bool Put(const char *code, bool repeat = false, bool release = false) {
	return cRemote::Put(code, repeat, release);
    }
};

/**
**	Player class.
*/
class cMyPlayer:public cPlayer
{
  private:
    char *FileName;			///< file to play

  public:
     cMyPlayer(const char *);		///< player constructor
     virtual ~ cMyPlayer();		///< player destructor
    void Activate(bool);		///< player attached/detached
    /// get current replay mode
    virtual bool GetReplayMode(bool &, bool &, int &);
};

/**
**	Status class.
**
**	To get volume changes.
*/
class cMyStatus:public cStatus
{
  private:
    int Volume;				///< current volume

  public:
    cMyStatus(void);			///< my status constructor

  protected:
    virtual void SetVolume(int, bool);	///< volume changed
};

/**
**	Our player control class.
*/
class cMyControl:public cControl
{
  private:
    cMyPlayer * Player;			///< our player
    cSkinDisplayReplay *Display;	///< our osd display
    void ShowReplayMode(void);		///< display replay mode
    void ShowProgress(void);		///< display progress bar
    virtual void Show(void);		///< show replay control
    virtual void Hide(void);		///< hide replay control
    bool infoVisible;			///< RecordingInfo visible
    time_t timeoutShow;			///< timeout shown control

  public:
    cMyControl(const char *);		///< player control constructor
    virtual ~ cMyControl();		///< player control destructor

    virtual eOSState ProcessKey(eKeys);	///< handle keyboard input

};

/*
 *	Plex Browser 
 */

class cPlexBrowser :public cOsdMenu
{
private:
	plexclient::Plexservice* pService;
	plexclient::MediaContainer *pCont;
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
	cPlexBrowser(const char *title, plexclient::Plexservice* pServ);
	
	/// File browser destructor
    //virtual ~ cPlexBrowser();
    /// Process keyboard input
    virtual eOSState ProcessKey(eKeys);
	
};

/**
**	Play plugin menu class.
*/
class cPlayMenu:public cOsdMenu
{
  private:
  public:
    cPlayMenu(const char *, plexclient::plexgdm* gdm, int = 0, int = 0, int = 0, int = 0, int = 0);
    virtual ~ cPlayMenu();
    virtual eOSState ProcessKey(eKeys);
};

/**
**	My device plugin OSD class.
*/
class cMyOsd:public cOsd
{
  public:
    static volatile char Dirty;		///< flag force redraw everything
    int OsdLevel;			///< current osd level

    cMyOsd(int, int, uint);		///< osd constructor
    virtual ~ cMyOsd(void);		///< osd destructor
    virtual void Flush(void);		///< commits all data to the hardware
    virtual void SetActive(bool);	///< sets OSD to be the active one
};

/**
**	My device plugin OSD provider class.
*/
class cMyOsdProvider:public cOsdProvider
{
  private:
    static cOsd *Osd;

  public:
    virtual cOsd * CreateOsd(int, int, uint);
    virtual bool ProvidesTrueColor(void);
    cMyOsdProvider(void);
};

/**
**	Dummy device class.
*/
class cMyDevice:public cDevice
{
  public:
    cMyDevice(void);
    virtual ~ cMyDevice(void);

    virtual void GetOsdSize(int &, int &, double &);

  protected:
    virtual void MakePrimaryDevice(bool);
};

class cMyPlugin:public cPlugin
{
  public:
    cMyPlugin(void);
    virtual ~ cMyPlugin(void);
    virtual const char *Version(void);
    virtual const char *Description(void);
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int, char *[]);
    virtual bool Initialize(void);
    virtual void MainThreadHook(void);
    virtual const char *MainMenuEntry(void);
    virtual cOsdObject *MainMenuAction(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *, const char *);
    virtual bool Service(const char *, void * = NULL);
    virtual const char **SVDRPHelpPages(void);
    virtual cString SVDRPCommand(const char *, const char *, int &);
	
  private:
	plexclient::Plexservice* pService;
	plexclient::plexgdm* pPlexgdm;
};

#endif