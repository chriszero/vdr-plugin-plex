#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"

static const char *const PLUGINNAME = "plex";
//////////////////////////////////////////////////////////////////////////////


    /// vdr-plugin description.
static const char *const DESCRIPTION = trNOOP("Plex for VDR Plugin");

    /// vdr-plugin text of main menu entry
static const char *MAINMENUENTRY = trNOOP("Plex for VDR");

//////////////////////////////////////////////////////////////////////////////

static char ConfigHideMainMenuEntry;	///< hide main menu entry
char ConfigDisableRemote;		///< disable remote during external play

static volatile int DoMakePrimary;	///< switch primary device to this

//////////////////////////////////////////////////////////////////////////////
//	C Callbacks
//////////////////////////////////////////////////////////////////////////////


/**
**	Feed key press as remote input (called from C part).
**
**	@param keymap	target keymap "XKeymap" name
**	@param key	pressed/released key name
**	@param repeat	repeated key flag
**	@param release	released key flag
*/
extern "C" void FeedKeyPress(const char *keymap, const char *key, int repeat,
    int release)
{
    cRemote *remote;
    cMyRemote *csoft;

    if (!keymap || !key) {
	return;
    }
    // find remote
    for (remote = Remotes.First(); remote; remote = Remotes.Next(remote)) {
	if (!strcmp(remote->Name(), keymap)) {
	    break;
	}
    }
    // if remote not already exists, create it
    if (remote) {
	csoft = static_cast<cMyRemote*>(remote);
    } else {
	dsyslog("[plex]%s: remote '%s' not found\n", __FUNCTION__, keymap);
	csoft = new cMyRemote(keymap);
    }

    //dsyslog("[plex]%s %s, %s\n", __FUNCTION__, keymap, key);
    if (key[1]) {			// no single character
	csoft->Put(key, repeat, release);
    } else if (!csoft->Put(key, repeat, release)) {
	cRemote::Put(KBDKEY(key[0]));	// feed it for edit mode
    }
}

/**
**	Disable remotes.
*/
void RemoteDisable(void)
{
    dsyslog("[plex]: remote disabled\n");
    cRemote::SetEnabled(false);
}

/**
**	Enable remotes.
*/
void RemoteEnable(void)
{
    dsyslog("[plex]: remote enabled\n");
    cRemote::SetEnabled(false);
    cRemote::SetEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////
//	C Callbacks for diashow
//////////////////////////////////////////////////////////////////////////////

#if 0

/**
**	Draw rectangle.
*/
extern "C" void DrawRectangle(int x1, int y1, int x2, int y2, uint32_t argb)
{
    //GlobalDiashow->Osd->DrawRectangle(x1, y1, x2, y2, argb);
}

/**
**	Draw text.
**
**	@param FIXME:
*/
extern "C" void DrawText(int x, int y, const char *s, uint32_t fg, uint32_t bg,
    int w, int h, int align)
{
    const cFont *font;

    font = cFont::GetFont(fontOsd);
    //GlobalDiashow->Osd->DrawText(x, y, s, fg, bg, font, w, h, align);
}

#endif


/**
**	Play a file.
**
**	@param filename	path and file name
*/
static void PlayFile(const char *filename)
{
    dsyslog("[plex]: play file '%s'\n", filename);
    //cControl::Launch(new cMyControl(filename));
	cControl::Launch(new cHlsPlayerControl(new cHlsPlayer(filename), "Test Title"));
}

//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////


static char ShowBrowser;		///< flag show browser
static plexclient::PlexServer* pPlexServer;
/*
static const char *BrowserStartDir;	///< browser start directory
static const NameFilter *BrowserFilters;	///< browser name filters
static int DirStackSize;		///< size of directory stack
static int DirStackUsed;		///< entries used of directory stack
static char **DirStack;			///< current path directory stack

*/


cPlexBrowser::cPlexBrowser(const char *title, plexclient::PlexServer* pServ) :cOsdMenu(title) {
		
	dsyslog("[plex]%s:\n", __FUNCTION__);
	pService = new plexclient::Plexservice(pServ);
	pService->Authenticate();
	pCont = pService->GetAllSections();	
	CreateMenu();
}

cPlexBrowser::~cPlexBrowser() {
	delete pService;
}

void cPlexBrowser::CreateMenu() {
	// Clear Menu
	Clear();
	// Directory or Video?
	if(pCont->m_vDirectories.size() > 0) {
		
		for(std::vector<plexclient::Directory>::iterator it = pCont->m_vDirectories.begin(); it != pCont->m_vDirectories.end(); ++it) {
			plexclient::Directory *pDir = &(*it);
			Add(new cPlexOsdItem( pDir->m_sTitle.c_str(), pDir) );
		}
	} 
	
	if(pCont->m_vVideos.size() > 0) {
		for(std::vector<plexclient::Video>::iterator it = pCont->m_vVideos.begin(); it != pCont->m_vVideos.end(); ++it) {
			plexclient::Video *vid = &(*it); // cast raw pointer
			Add(new cPlexOsdItem( vid->m_sTitle.c_str(), vid) );
		}	
	}
	
	if(Count() < 1) {
		Add(new cPlexOsdItem("Empty"));
	}
	
	Display();
}


eOSState cPlexBrowser::ProcessKey(eKeys key) {
	eOSState state;

    // call standard function
    state = cOsdMenu::ProcessKey(key);
    if (state || key != kNone) {
		dsyslog("[plex]%s: state=%d key=%d\n", __FUNCTION__, state, key);
    }

    switch (state) {
	case osUnknown:
	    switch (key) {
		case kOk:
		    return ProcessSelected();
		case kBack:
			return LevelUp();
		default:
		    break;
	    }
	    break;
	case osBack:
	    state = LevelUp();
	    if (state == osEnd) {	// top level reached
			return osPlugin;
	    }
	default:
	    break;
    }
    return state;
}

eOSState cPlexBrowser::LevelUp() {
	if(m_vStack.size() <= 1){
		m_vStack.clear();
		ShowBrowser = 0;
		return osEnd;
	}
	
	m_vStack.pop_back();
	
	std::string uri;
	for(unsigned int i = 0; i < m_vStack.size(); i++) {
			if(m_vStack[i][0]=='/') {
				uri = m_vStack[i];
				continue;
			}
			uri += m_vStack[i];
			if(i+1 < m_vStack.size()) {
				uri += "/";
			}
		}
	std::cout << "m_sSection: " << uri << std::endl;
	
	pCont = pService->GetSection(uri);
	
	CreateMenu();
	return osContinue;
}

eOSState cPlexBrowser::ProcessSelected() {
    std::string fullUri;
    //char *filename;
    //char *tmp;

    int current = Current();		// get current menu item index
    cPlexOsdItem *item = static_cast<cPlexOsdItem*>(Get(current));
	
	
	if(item->IsVideo()) {
		plexclient::Video* pVid = item->GetAttachedVideo();
		fullUri = pService->GetServer()->GetUri() + pVid->m_pMedia->m_sPartKey;		
		//PlayFile(fullUri.c_str());
		//std::cout << "TrancodeUri: " << pService->GetTranscodeUrl(pVid) << std::endl;
		std::cout << "TrancodeUri: " << pService->GetUniversalTranscodeUrl(pVid) << std::endl;
		
		PlayFile(pService->GetUniversalTranscodeUrl(pVid).c_str());
		return osEnd;
	}
	
	
	if(item->IsDir()) {
		plexclient::Directory* pDir = item->GetAttachedDirectory();
		
		m_vStack.push_back(pDir->m_sKey);
		std::string uri;
		for(unsigned int i = 0; i < m_vStack.size(); i++) {
			if(m_vStack[i][0]=='/') {
				uri = m_vStack[i];
				continue;
			}
			uri += m_vStack[i];
			if(i+1 < m_vStack.size()) {
				uri += "/";
			}
		}
		std::cout << "m_sSection: " << uri << std::endl;
		
		pCont = pService->GetSection(uri);
		
		CreateMenu();
		return osContinue;
	}
	
	//return osEnd;
    return osContinue;
}


//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////

/**
**	Play menu constructor.
*/
cPlayMenu::cPlayMenu(const char *title, int c0, int c1, int c2, int c3, int c4)
:cOsdMenu(title, c0, c1, c2, c3, c4)
{
	SetHasHotkeys();
	
	for(std::vector<plexclient::PlexServer>::iterator it = plexclient::plexgdm::GetInstance().GetPlexservers().begin(); it != plexclient::plexgdm::GetInstance().GetPlexservers().end(); ++it) {
		plexclient::PlexServer *pServer = &(*it);
		Add(new cPlexOsdItem(pServer->GetServerName().c_str(), pServer));
	}
	
	if(Count() < 1) {
		Add(new cPlexOsdItem("No Plex Media Server found."), false);
	}
}

/**
**	Play menu destructor.
*/
cPlayMenu::~cPlayMenu()
{
}

/**
**	Handle play plugin menu key event.
**
**	@param key	key event
*/
eOSState cPlayMenu::ProcessKey(eKeys key)
{
    eOSState state;

    if (key != kNone) {
	dsyslog("[plex]%s: key=%d\n", __FUNCTION__, key);
    }
    // call standard function
    state = cOsdMenu::ProcessKey(key);
	
	int current = Current();		// get current menu item index
	cPlexOsdItem *item = static_cast<cPlexOsdItem*>(Get(current));
	
    switch (state) {
		case osUnknown:
			switch (key) {
				case kOk:
					pPlexServer = item->GetAttachedServer();
					ShowBrowser = 1;
					return osPlugin;		// restart with OSD browser
				default:
					break;
			}
		default:
			break;
    }
    return state;
}

//////////////////////////////////////////////////////////////////////////////
//	cPlugin
//////////////////////////////////////////////////////////////////////////////

/**
**	Initialize any member variables here.
**
**	@note DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
**	VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
*/
cMyPlugin::cMyPlugin(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
}

/**
**	Clean up after yourself!
*/
cMyPlugin::~cMyPlugin(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
	plexclient::plexgdm::GetInstance().stopRegistration();
	plexclient::ControlServer::GetInstance().Stop();
}

/**
**	Return plugin version number.
**
**	@returns version number as constant string.
*/
const char *cMyPlugin::Version(void)
{
    return VERSION;
}

/**
**	Return plugin short description.
**
**	@returns short description as constant string.
*/
const char *cMyPlugin::Description(void)
{
    return tr(DESCRIPTION);
}

/**
**	Return a string that describes all known command line options.
**
**	@returns command line help as constant string.
*/
const char *cMyPlugin::CommandLineHelp(void)
{
}

/**
**	Process the command line arguments.
*/
bool cMyPlugin::ProcessArgs(int argc, char *argv[])
{
}

/**
**	Start any background activities the plugin shall perform.
*/
bool cMyPlugin::Initialize(void)
{
    //dsyslog("[plex]%s:\n", __FUNCTION__);

    // FIXME: can delay until needed?
    //Status = new cMyStatus;		// start monitoring
    // FIXME: destructs memory
	
	// First Startup? Save UUID 
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());
	
	plexclient::plexgdm::GetInstance().discover();
	
	if (plexclient::plexgdm::GetInstance().GetPlexservers().size() > 0) {
		plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(), DESCRIPTION, "3200", "VDR", VERSION);
		plexclient::plexgdm::GetInstance().Start();
		
		plexclient::ControlServer::GetInstance().Start();
		
	} else {
		perror("No Plexserver found");
		std::cout << "No Plexmediaserver found" << std::endl;
	}

    return true;
}

/**
**	Create main menu entry.
*/
const char *cMyPlugin::MainMenuEntry(void)
{
    return ConfigHideMainMenuEntry ? NULL : tr(MAINMENUENTRY);
}

/**
**	Perform the action when selected from the main VDR menu.
*/
cOsdObject *cMyPlugin::MainMenuAction(void)
{
    //dsyslog("[plex]%s:\n", __FUNCTION__);

    if (ShowBrowser) {
		return new cPlexBrowser("Newest", pPlexServer);
    }
    return new cPlayMenu("Plex");
}

/**
**	Receive requests or messages.
**
**	@param id	unique identification string that identifies the
**			service protocol
**	@param data	custom data structure
*/
bool cMyPlugin::Service(const char *id, void *data)
{
}

/**
**	Return SVDRP commands help pages.
**
**	return a pointer to a list of help strings for all of the plugin's
**	SVDRP commands.
*/
const char **cMyPlugin::SVDRPHelpPages(void)
{
	return NULL;
}

/**
**	Handle SVDRP commands.
**
**	@param command		SVDRP command
**	@param option		all command arguments
**	@param reply_code	reply code
*/
cString cMyPlugin::SVDRPCommand(const char *command, const char *option, int &reply_code)
{
    return NULL;
}

/**
**	Called for every plugin once during every cycle of VDR's main program
**	loop.
*/
void cMyPlugin::MainThreadHook(void)
{
    // dsyslog("[plex]%s:\n", __FUNCTION__);

    if (DoMakePrimary) {
	dsyslog("[plex]: switching primary device to %d\n", DoMakePrimary);
	cDevice::SetPrimaryDevice(DoMakePrimary);
	DoMakePrimary = 0;
    }
	// Start Tasks, e.g. Play Video
	if(plexclient::ActionManager::GetInstance().IsAction()) {
		std::string file = plexclient::ActionManager::GetInstance().GetAction();
		PlayFile(file.c_str());
	}
}

/**
**	Return our setup menu.
*/
cMenuSetupPage *cMyPlugin::SetupMenu(void)
{
    //dsyslog("[plex]%s:\n", __FUNCTION__);

    return new cMyMenuSetupPage;
}

/**
**	Parse setup parameters
**
**	@param name	paramter name (case sensetive)
**	@param value	value as string
**
**	@returns true if the parameter is supported.
*/
bool cMyPlugin::SetupParse(const char *name, const char *value)
{
    //dsyslog("[plex]%s: '%s' = '%s'\n", __FUNCTION__, name, value);

    if (strcasecmp(name, "HideMainMenuEntry") == 0) 	Config::GetInstance().HideMainMenuEntry = atoi(value) ? true : false;
	else if (strcasecmp(name, "DisableRemote") == 0) 	Config::GetInstance().DisableRemote = atoi(value) ? true : false;
	else if (strcasecmp(name, "Username") == 0) 		Config::GetInstance().s_username = std::string(value);
	else if (strcasecmp(name, "Password") == 0) 		Config::GetInstance().s_password = std::string(value);
	else if (strcasecmp(name, "UUID") == 0) 			Config::GetInstance().SetUUID(value);
	else return false;

    return true;
}

VDRPLUGINCREATOR(cMyPlugin);		// Don't touch this!
