#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"
#include "plexOsd.h"
#include "plexSdOsd.h"
#include "pictureCache.h"
#include "services.h"

#include <libskindesignerapi/skindesignerapi.h>

//////////////////////////////////////////////////////////////////////////////
//	cPlugin
//////////////////////////////////////////////////////////////////////////////

volatile bool cMyPlugin::CalledFromCode = false;
bool cMyPlugin::bSkindesigner = false;

/**
**	Initialize any member variables here.
**
**	@note DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
**	VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
*/
cMyPlugin::cMyPlugin(void)
{
	m_pSdCheck = NULL;
}

/**
**	Clean up after yourself!
*/
cMyPlugin::~cMyPlugin(void)
{
	plexclient::plexgdm::GetInstance().stopRegistration();
	plexclient::ControlServer::GetInstance().Stop();
	delete m_pSdCheck;
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


bool cMyPlugin::Start(void)
{
	skindesignerapi::cPluginStructure reg;
	reg.name = "plex";
	reg.libskindesignerAPIVersion = LIBSKINDESIGNERAPIVERSION;

	reg.SetView(viRootView, "root.xml");
	reg.SetViewGrid(eViews::viRootView, eViewGrids::vgCover, "coverbrowser");
	reg.SetViewGrid(eViews::viRootView, eViewGrids::vgList, "listbrowser");
	reg.SetViewGrid(eViews::viRootView, eViewGrids::vgDetail, "detailbrowser");
	reg.SetViewElement(viRootView, verHeader, "header");
	reg.SetViewElement(viRootView, verBackground, "background");
	reg.SetViewElement(viRootView, verInfopane, "infopane");
	reg.SetViewElement(viRootView, verFooter, "footer");
	reg.SetViewElement(viRootView, verWatch, "time");
	reg.SetViewElement(viRootView, verMessage, "message");
	reg.SetViewElement(viRootView, verScrollbar, "scrollbar");
	/*
		reg.SetSubView(viRootView, viDetailView, "detail.xml");
		reg.SetViewElement(viDetailView, vedBackground, "background");
		reg.SetViewElement(viDetailView, vedHeader, "header");
		reg.SetViewElement(viDetailView, vedFooter, "footer");
	*/
	
	//reg.SetMenu(meRoot, "streamselect.xml");
	if (skindesignerapi::SkindesignerAPI::RegisterPlugin(&reg)) {
		m_pSdCheck = new cPlexSdOsd();
		cMyPlugin::bSkindesigner = m_pSdCheck->SdSupport();
	} else {
		esyslog("[plex]: %s skindesigner not available", __FUNCTION__);
	}
	return true;
}

/**
**	Start any background activities the plugin shall perform.
*/
bool cMyPlugin::Initialize(void)
{
	// First Startup? Save UUID
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());

	plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(), Config::GetInstance().GetHostname(), "3200", DESCRIPTION, VERSION);
	plexclient::plexgdm::GetInstance().Start();
	plexclient::ControlServer::GetInstance().Start();
	cPictureCache::GetInstance().Start();

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
	if(m_pSdCheck && m_pSdCheck->SdSupport()) return new cPlexSdOsd();
	else return cPlexMenu::ProcessMenu();
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

	return Config::GetInstance().Parse(name, value);
}

/**
**	Play a file.
**
**	@param filename	path and file name
*/
void cMyPlugin::PlayFile(plexclient::Video Vid)
{
	cPlugin* mpvPlugin = cPluginManager::GetPlugin("mpv");

	if(Config::GetInstance().UseMpv && mpvPlugin) {
		Mpv_StartPlayService_v1_0_t req;
		char* file = (char*)(Vid.m_pServer->GetUri() + Vid.m_Media.m_sPartKey).c_str();
		req.Filename = file;
		req.Title = (char*)Vid.GetTitle().c_str();
		//req.Title = &Vid.GetTitle().c_str();
		mpvPlugin->Service(MPV_START_PLAY_SERVICE, &req);
		return;

	} else if (Config::GetInstance().UseMpv) {
		isyslog("Can't find mpv %s, playing directly.", mpvPlugin ? "service" : "plugin");
	}


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
