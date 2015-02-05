#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"

static const char *DESCRIPTION = "Plex for VDR Plugin";
static const char *MAINMENUENTRY = "Plex for VDR";

//////////////////////////////////////////////////////////////////////////////

//static char ConfigHideMainMenuEntry;	///< hide main menu entry
//char ConfigDisableRemote;		///< disable remote during external play

/**
**	Play a file.
**
**	@param filename	path and file name
*/
static void PlayFile(plexclient::Video* pVid)
{
	isyslog("[plex]: play file '%s'\n", pVid->m_sKey.c_str());
	cControl* control = cHlsPlayerControl::Create(pVid);
	if(control) {
		cControl::Launch(control);
	}
}

//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////


static char ShowBrowser;		///< flag show browser
static plexclient::PlexServer* pPlexServer;

cPlexBrowser::cPlexBrowser(const char *title, plexclient::PlexServer* pServ) :cOsdMenu(title)
{

	dsyslog("[plex]%s:\n", __FUNCTION__);
	pService = new plexclient::Plexservice(pServ);
	pService->Authenticate();
	pCont = pService->GetAllSections();
	CreateMenu();
}

cPlexBrowser::~cPlexBrowser()
{
	delete pService;
}

void cPlexBrowser::CreateMenu()
{
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


eOSState cPlexBrowser::ProcessKey(eKeys key)
{
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

eOSState cPlexBrowser::LevelUp()
{
	if(m_vStack.size() <= 1) {
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

	pCont = pService->GetSection(uri);

	CreateMenu();
	return osContinue;
}

eOSState cPlexBrowser::ProcessSelected()
{
	int current = Current();		// get current menu item index
	cPlexOsdItem *item = static_cast<cPlexOsdItem*>(Get(current));


	if(item->IsVideo()) {
		plexclient::Video* pVid = item->GetAttachedVideo();
		PlayFile(pVid);
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
	return DESCRIPTION;
}

/**
**	Start any background activities the plugin shall perform.
*/
bool cMyPlugin::Initialize(void)
{
	// First Startup? Save UUID
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());

	plexclient::plexgdm::GetInstance().discover();

	if (plexclient::plexgdm::GetInstance().GetPlexservers().size() > 0) {
		plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(), DESCRIPTION, "3200", "VDR", VERSION);
		plexclient::plexgdm::GetInstance().Start();

		plexclient::ControlServer::GetInstance().Start();

	} else {
		esyslog("[plex]No Plexmediaserver found");
	}

	return true;
}

/**
**	Create main menu entry.
*/
const char *cMyPlugin::MainMenuEntry(void)
{
	return Config::GetInstance().HideMainMenuEntry ? NULL : MAINMENUENTRY;
}

/**
**	Perform the action when selected from the main VDR menu.
*/
cOsdObject *cMyPlugin::MainMenuAction(void)
{
	//dsyslog("[plex]%s:\n", __FUNCTION__);

	if (ShowBrowser) {
		return new cPlexBrowser("Browse Plex", pPlexServer);
	}
	return new cPlayMenu("Plex");
}

/**
**	Called for every plugin once during every cycle of VDR's main program
**	loop.
*/
void cMyPlugin::MainThreadHook(void)
{
	// dsyslog("[plex]%s:\n", __FUNCTION__);
	// Start Tasks, e.g. Play Video
	if(plexclient::ActionManager::GetInstance().IsAction()) {
		plexclient::Video* video = plexclient::ActionManager::GetInstance().GetAction();
		PlayFile(video);
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
	else if (strcasecmp(name, "Username") == 0) 		Config::GetInstance().s_username = std::string(value);
	else if (strcasecmp(name, "Password") == 0) 		Config::GetInstance().s_password = std::string(value);
	else if (strcasecmp(name, "UUID") == 0) 			Config::GetInstance().SetUUID(value);
	else return false;

	return true;
}

VDRPLUGINCREATOR(cMyPlugin);		// Don't touch this!
