#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"


//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////


static char ShowBrowser;		///< flag show browser
static std::shared_ptr<plexclient::Plexservice> pPlexService;

std::shared_ptr<plexclient::Plexservice> cPlexBrowser::pLastService;
int cPlexBrowser::lastCurrentItem;

cPlexBrowser::cPlexBrowser(const char *title, std::shared_ptr<plexclient::Plexservice> Service) :cOsdMenu(title)
{
	dsyslog("[plex]%s:\n", __FUNCTION__);
	pService = Service;
	pService->Authenticate();
	if(pService == pLastService) {
		pCont = pService->GetLastSection(true);
	} else {
		pCont = pService->GetSection(pService->StartUri);
	}
	SetMenuCategory(mcRecording);
	CreateMenu();
}

cPlexBrowser* cPlexBrowser::RecoverLastState()
{
	if (cPlexBrowser::pLastService != NULL) {
		cPlexBrowser* pBrowser = new cPlexBrowser("", cPlexBrowser::pLastService);
		return pBrowser;
	}
	return NULL;
}

void cPlexBrowser::CreateMenu()
{
	// Clear Menu
	Clear();
	// Directory or Video?
	if(pCont && pCont->m_vDirectories.size() > 0) {

		for(std::vector<plexclient::Directory>::iterator it = pCont->m_vDirectories.begin(); it != pCont->m_vDirectories.end(); ++it) {
			plexclient::Directory *pDir = &(*it);
			Add(new cPlexOsdItem( pDir->GetTitle().c_str(), pDir) );
		}
	}

	if(pCont && pCont->m_vVideos.size() > 0) {
		for(std::vector<plexclient::Video>::iterator it = pCont->m_vVideos.begin(); it != pCont->m_vVideos.end(); ++it) {
			plexclient::Video *vid = &(*it); // cast raw pointer
			Add(new cPlexOsdItem( vid->GetTitle().c_str(), vid) );
		}
	}

	if(Count() < 1) {
		Add(new cPlexOsdItem("Empty"));
	} else if (pService == pLastService) {
		// recover last selected item
		cOsdItem* item = Get(lastCurrentItem);
		SetCurrent(item);
		pLastService = NULL;
	}

	Display();
}

eOSState cPlexBrowser::ProcessKey(eKeys key)
{
	eOSState state;

	// call standard function
	state = cOsdMenu::ProcessKey(key);
	//if (state || key != kNone) {
	//	dsyslog("[plex]%s: state=%d key=%d\n", __FUNCTION__, state, key);
	//}

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
	pCont = pService->GetLastSection();
	if(!pCont) {
		ShowBrowser = 0;
		return osEnd;
	}
	CreateMenu();
	return osContinue;
}

eOSState cPlexBrowser::ProcessSelected()
{
	int current = Current();		// get current menu item index
	cPlexOsdItem *item = static_cast<cPlexOsdItem*>(Get(current));


	if(item->IsVideo()) {
		pLastService = pService;
		lastCurrentItem = current;
		cMyPlugin::PlayFile(*item->GetAttachedVideo());
		return osEnd;
	}

	if(item->IsDir()) {
		plexclient::Directory* pDir = item->GetAttachedDirectory();
		pCont = pService->GetSection(pDir->m_sKey);
		//SetTitle(pDir->m_sTitle);
		CreateMenu();
		return osContinue;
	}

	//return osEnd;
	return osContinue;
}


cPlexInfo::cPlexInfo(plexclient::Video* video) : cOsdMenu(video->GetTitle().c_str())
{
	cOsdMenu::Display();

	Add(new cOsdItem(video->m_sSummary.c_str()));
}

eOSState cPlexInfo::ProcessKey(eKeys Key)
{
	switch (int(Key)) {
	case kUp|k_Repeat:
	case kUp:
	case kDown|k_Repeat:
	case kDown:
	case kLeft|k_Repeat:
	case kLeft:
	case kRight|k_Repeat:
	case kRight:
		DisplayMenu()->Scroll(NORMALKEY(Key) == kUp || NORMALKEY(Key) == kLeft, NORMALKEY(Key) == kLeft || NORMALKEY(Key) == kRight);
		cStatus::MsgOsdTextItem(NULL, NORMALKEY(Key) == kUp || NORMALKEY(Key) == kLeft);
		return osContinue;
	case kInfo:
		return osBack;
	default:
		break;
	}

	eOSState state = cOsdMenu::ProcessKey(Key);

	if (state == osUnknown) {
		switch (Key) {
		case kGreen:
			cRemote::Put(Key, true);
		case kOk:
			return osBack;
		default:
			break;
		}
	}
	return state;
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
		//&(*it)
		auto s1 = std::make_shared<plexclient::Plexservice>( &(*it) );
		s1->StartUri = "/library/sections";
		Add(new cPlexOsdItem(Poco::format("%s - Library", it->GetServerName()).c_str(),  s1));

		auto s2 = std::make_shared<plexclient::Plexservice>( &(*it) );
		s2->StartUri = "/video";
		Add(new cPlexOsdItem(Poco::format("%s - Video Channels", it->GetServerName()).c_str(), s2 ));
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

	//if (key != kNone) {
	//	dsyslog("[plex]%s: key=%d\n", __FUNCTION__, key);
	//}
	// call standard function
	state = cOsdMenu::ProcessKey(key);

	int current = Current();		// get current menu item index
	cPlexOsdItem *item = static_cast<cPlexOsdItem*>(Get(current));

	switch (state) {
	case osUnknown:
		switch (key) {
		case kOk:
			pPlexService = item->GetAttachedService();
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

volatile bool cMyPlugin::CalledFromCode = false;

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

	plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(), DESCRIPTION, "3200", "VDR", VERSION);
	plexclient::plexgdm::GetInstance().Start();
	plexclient::ControlServer::GetInstance().Start();

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

	if(CalledFromCode) {
		CalledFromCode = false;
		return cPlexBrowser::RecoverLastState();
	}

	if (ShowBrowser) {
		return new cPlexBrowser("Browse Plex", pPlexService);
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
		PlayFile(plexclient::ActionManager::GetInstance().GetAction());
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
	else if (strcasecmp(name, "UsePlexAccount") == 0) 	Config::GetInstance().UsePlexAccount = atoi(value) ? true : false;
	else if (strcasecmp(name, "UseCustomTranscodeProfile") == 0) 	Config::GetInstance().UseCustomTranscodeProfile = atoi(value) ? true : false;
	else if (strcasecmp(name, "Username") == 0) 		Config::GetInstance().s_username = std::string(value);
	else if (strcasecmp(name, "Password") == 0) 		Config::GetInstance().s_password = std::string(value);
	else if (strcasecmp(name, "UUID") == 0) 			Config::GetInstance().SetUUID(value);
	else return false;

	return true;
}

/**
**	Play a file.
**
**	@param filename	path and file name
*/
void cMyPlugin::PlayFile(plexclient::Video Vid)
{
	isyslog("[plex]: play file '%s'\n", Vid.m_sKey.c_str());
	if(Vid.m_iMyPlayOffset == 0 && Vid.m_lViewoffset > 0 ) {
		cString message = cString::sprintf(tr("To start from %ld minutes, press Ok."), Vid.m_lViewoffset / 60000);
		eKeys response = Skins.Message(eMessageType::mtInfo, message, 5);
		if(response == kOk) {
			Vid.m_iMyPlayOffset = Vid.m_lViewoffset/1000;
		}
	}
	cControl* control = cHlsPlayerControl::Create(Vid);
	if(control) {
		cControl::Launch(control);
	}
}

VDRPLUGINCREATOR(cMyPlugin);		// Don't touch this!
