#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"

extern "C"
{
#include "video.h"
#include "player.h"
}

static const char *const PLUGINNAME = "plex";
//////////////////////////////////////////////////////////////////////////////


    /// vdr-plugin description.
static const char *const DESCRIPTION = trNOOP("VDR Plex Plugin");

    /// vdr-plugin text of main menu entry
static const char *MAINMENUENTRY = trNOOP("Plex");

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

//////////////////////////////////////////////////////////////////////////////
//	cPlayer
//////////////////////////////////////////////////////////////////////////////


/**
**	Player constructor.
**
**	@param filename	path and name of file to play
*/
cMyPlayer::cMyPlayer(const char *filename)
:cPlayer(pmExtern_THIS_SHOULD_BE_AVOIDED)
{
    dsyslog("[plex]%s: '%s'\n", __FUNCTION__, filename);

    PlayerSetVolume(cDevice::CurrentVolume());
    dsyslog("[plex]: initial volume %d\n", cDevice::CurrentVolume());

    FileName = strdup(filename);
    if (ConfigDisableRemote) {
	RemoteDisable();
    }
}

/**
**	Player destructor.
*/
cMyPlayer::~cMyPlayer()
{
    dsyslog("[plex]%s: end\n", __FUNCTION__);

    PlayerStop();
    free(FileName);
    if (ConfigDisableRemote) {
	RemoteEnable();
    }
    // FIXME: wait until primary device is switched?
    dsyslog("[plex]: device %d->%d\n",
	cDevice::PrimaryDevice()->DeviceNumber(), DoMakePrimary);
}

/**
**	Player attached or detached.
**
**	@param on	flag turn player on or off
*/
void cMyPlayer::Activate(bool on)
{
    dsyslog("[plex]%s: '%s' %d\n", __FUNCTION__, FileName, on);

    if (on) {
	PlayerStart(FileName);
    } else {
	PlayerStop();
    }
}

/**
**	Get current replay mode.
*/
bool cMyPlayer::GetReplayMode(bool & play, bool & forward, int &speed)
{
    play = !PlayerPaused;
    forward = true;
    if (PlayerSpeed == 1) {
	speed = -1;
    } else {
	speed = PlayerSpeed;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//	cStatus
//////////////////////////////////////////////////////////////////////////////

cMyStatus *Status;			///< status monitor for volume

/**
**	Status constructor.
*/
cMyStatus::cMyStatus(void)
{
    Volume = cDevice::CurrentVolume();

    dsyslog("[plex]: status volume %d\n", Volume);
}

/**
**	Called if volume is set.
*/
void cMyStatus::SetVolume(int volume, bool absolute)
{
    dsyslog("[plex]: volume %d %s\n", volume, absolute ? "abs" : "rel");

    if (absolute) {
	Volume = volume;
    } else {
	Volume += volume;
    }

    PlayerSetVolume(Volume);
}

//////////////////////////////////////////////////////////////////////////////
//	cControl
//////////////////////////////////////////////////////////////////////////////



/**
**	Show replay mode.
*/
void cMyControl::ShowReplayMode(void)
{
    dsyslog("[plex]%s: %d - %d\n", __FUNCTION__, Setup.ShowReplayMode,
	cOsd::IsOpen());

    // use vdr setup
    if (Display || (Setup.ShowReplayMode && !cOsd::IsOpen())) {
	bool play;
	bool forward;
	int speed;

	if (GetReplayMode(play, forward, speed)) {
	    if (!Display) {
		// no need to show normal play
		if (play && forward && speed == 1) {
		    return;
		}
		Display = Skins.Current()->DisplayReplay(true);
	    }
	    Display->SetMode(play, forward, speed);
	}
    }
}

/**
**	Show progress.
*/
void cMyControl::ShowProgress(void)
{
    if (Display || (!cOsd::IsOpen())) {
	bool play;
	bool forward;
	int speed;

	if (GetReplayMode(play, forward, speed)) {
	    if (!Display) {
		Display = Skins.Current()->DisplayReplay(false);
	    }

	    if (!infoVisible) {
		infoVisible = true;
		timeoutShow = time(0) + Setup.ChannelInfoTime;
		PlayerGetLength();
		PlayerGetMetaTitle();
		PlayerGetFilename();
	    }

	    PlayerGetCurrentPosition();
	    if (strcmp(PlayerTitle, "") != 0) {
		Display->SetTitle(PlayerTitle);
	    } else {
		Display->SetTitle(PlayerFilename);
	    }
	    Display->SetProgress(PlayerCurrent, PlayerTotal);
	    Display->SetMode(play, forward, speed);
	    Display->SetCurrent(IndexToHMSF(PlayerCurrent, false, 1));
	    Display->SetTotal(IndexToHMSF(PlayerTotal, false, 1));
	}
	SetNeedsFastResponse(true);
	Skins.Flush();
    }
}

/**
**	Show control.
*/
void cMyControl::Show(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
    if (Setup.ShowReplayMode)
	ShowReplayMode();
    else
	ShowProgress();
}

/**
**	Control constructor.
**
**	@param filename	pathname of file to play.
*/
cMyControl::cMyControl(const char *filename)
:cControl(Player = new cMyPlayer(filename))
{
    Display = NULL;
    Status = new cMyStatus;		// start monitoring volume
    infoVisible = false;

    //LastSkipKey = kNone;
    //LastSkipSeconds = REPLAYCONTROLSKIPSECONDS;
    //LastSkipTimeout.Set(0);
    cStatus::MsgReplaying(this, filename, filename, true);

    cDevice::PrimaryDevice()->ClrAvailableTracks(true);
}

/**
**	Control destructor.
*/
cMyControl::~cMyControl()
{
    dsyslog("[plex]%s\n", __FUNCTION__);

    delete Player;

    //delete Display;
    delete Status;

    Hide();
    cStatus::MsgReplaying(this, NULL, NULL, false);
    //Stop();
}

/**
**	Hide control.
*/
void cMyControl::Hide(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);

    if (Display) {
	delete Display;

	Display = NULL;
	SetNeedsFastResponse(false);
    }
}

/**
**	Process keyboard input.
**
**	@param key	pressed or releaded key
*/
eOSState cMyControl::ProcessKey(eKeys key)
{
    eOSState state;

    if (key != kNone) {
	dsyslog("[plex]%s: key=%d\n", __FUNCTION__, key);
    }

    if (!PlayerIsRunning()) {		// check if player is still alive
	dsyslog("[plex]: player died\n");
	Hide();
	//FIXME: Stop();
	cControl::Shutdown();
	return osEnd;
    }

    if (infoVisible) {			// if RecordingInfo visible then update
	if (timeoutShow && time(0) > timeoutShow) {
	    Hide();
	    timeoutShow = 0;
	    infoVisible = false;
	} else
	    ShowProgress();
    }
    //state=cOsdMenu::ProcessKey(key);
    state = osContinue;
    switch ((int)key) {			// cast to shutup g++ warnings
	case kUp:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavUp();
		break;
	    }
	case kPlay:
	    Hide();
	    if (PlayerSpeed != 1) {
		PlayerSendSetSpeed(PlayerSpeed = 1);
	    }
	    if (PlayerPaused) {
		PlayerSendPause();
		PlayerPaused ^= 1;
	    }
	    Show();
	    break;

	case kDown:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavDown();
		break;
	    }
	case kPause:
	    PlayerSendPause();
	    PlayerPaused ^= 1;
	    Show();
	    break;

	case kFastRew | k_Release:
	case kLeft | k_Release:
	    if (Setup.MultiSpeedMode) {
		break;
	    }
	    // FIXME:
	    break;
	case kLeft:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavLeft();
		break;
	    }
	case kFastRew:
	    if (PlayerSpeed > 1) {
		PlayerSendSetSpeed(PlayerSpeed /= 2);
	    } else {
		PlayerSendSeek(-10);
	    }
	    Show();
	    break;
	case kRight:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavRight();
		break;
	    }
	case kFastFwd:
	    if (PlayerSpeed < 32) {
		PlayerSendSetSpeed(PlayerSpeed *= 2);
	    }
	    Show();
	    break;

	case kRed:
	    // FIXME: TimeSearch();
	    break;

#ifdef USE_JUMPINGSECONDS
	case kGreen | k_Repeat:
	    PlayerSendSeek(-Setup.JumpSecondsRepeat);
	    break;
	case kGreen:
	    PlayerSendSeek(-Setup.JumpSeconds);
	    break;
	case k1 | k_Repeat:
	case k1:
	    PlayerSendSeek(-Setup.JumpSecondsSlow);
	    break;
	case k3 | k_Repeat:
	case k3:
	    PlayerSendSeek(Setup.JumpSecondsSlow);
	    break;
	case kYellow | k_Repeat:
	    PlayerSendSeek(Setup.JumpSecondsRepeat);
	    break;
	case kYellow:
	    PlayerSendSeek(Setup.JumpSeconds);
	    break;
#else
	case kGreen | k_Repeat:
	case kGreen:
	    PlayerSendSeek(-60);
	    break;
	case kYellow | k_Repeat:
	case kYellow:
	    PlayerSendSeek(+60);
	    break;
#endif /* JUMPINGSECONDS */
#ifdef USE_LIEMIKUUTIO
#ifndef USE_JUMPINGSECONDS
	case k1 | k_Repeat:
	case k1:
	    PlayerSendSeek(-20);
	    break;
	case k3 | k_Repeat:
	case k3:
	    PlayerSendSeek(+20);
	    break;
#endif /* JUMPINGSECONDS */
#endif

	case kStop:
	case kBlue:
	    dsyslog("[plex]: player stopped\n");
	    Hide();
	    // FIXME: Stop();
	    cControl::Shutdown();
	    return osEnd;

	case kOk:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavSelect();
		// FIXME: PlayerDvdNav = 0;
		break;
	    }
	    if (infoVisible) {
		Hide();
		infoVisible = false;
	    } else
		Show();
	    break;

	case kBack:
	    if (PlayerDvdNav > 1) {
		PlayerSendDvdNavPrev();
		break;
	    }
	    PlayerSendQuit();
	    // FIXME: need to select old directory and index
	    // FIXME: this shows a half drawn OSD
	    cRemote::CallPlugin(PLUGINNAME);
	    return osBack;

	case kMenu:			// VDR: eats the keys
	case k5:
	    if (PlayerDvdNav) {
		PlayerSendDvdNavMenu();
		break;
	    }
	    break;

	case kAudio:			// VDR: eats the keys
	case k7:
	    // FIXME: audio menu
	    PlayerSendSwitchAudio();
	    break;
	case kSubtitles:		// VDR: eats the keys
	case k9:
	    // FIXME: subtitle menu
	    PlayerSendSubSelect();
	    break;

	default:
	    break;
    }

    return state;
}

/**
**	Play a file.
**
**	@param filename	path and file name
*/
static void PlayFile(const char *filename)
{
    dsyslog("[plex]: play file '%s'\n", filename);
    cControl::Launch(new cMyControl(filename));
}

//////////////////////////////////////////////////////////////////////////////
//	cOsdMenu
//////////////////////////////////////////////////////////////////////////////


static char ShowBrowser;		///< flag show browser
/*
static const char *BrowserStartDir;	///< browser start directory
static const NameFilter *BrowserFilters;	///< browser name filters
static int DirStackSize;		///< size of directory stack
static int DirStackUsed;		///< entries used of directory stack
static char **DirStack;			///< current path directory stack

*/


cPlexBrowser::cPlexBrowser(const char *title, plexclient::Plexservice* pServ) :cOsdMenu(title) {
		
	dsyslog("[plex]%s:\n", __FUNCTION__);
	pService = pServ;
	pCont = pService->GetAllSections();	
	CreateMenu();
}

void cPlexBrowser::CreateMenu() {
	// Clear Menu
	Clear();
	// Directory or Video?
	if(pCont->m_vDirectories.size() > 0) {
		v_Dir = &pCont->m_vDirectories;
		
		for(auto& pDir : pCont->m_vDirectories) {
			if(pDir.m_eType != plexclient::MediaType::MUSIC && pDir.m_eType != plexclient::MediaType::PHOTO) {
				Add(new cPlexOsdItem( pDir.m_sTitle.c_str(), &pDir) );
			}
		}
	} 
	

	if (pCont->m_vVideos.size() > 0) {
		for(auto& v_Vid : pCont->m_vVideos) {
			Add(new cPlexOsdItem( v_Vid.m_sTitle.c_str(), &v_Vid) );
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
	int current;
    cPlexOsdItem *item;
    std::string fullUri;
    //char *filename;
    //char *tmp;

    current = Current();		// get current menu item index
    item = static_cast<cPlexOsdItem*>(Get(current));
	
	
	if(item->IsVideo()) {
		plexclient::Video* pVid = item->GetAttachedVideo();
		fullUri = pService->GetServer()->GetUri() + pVid->m_pMedia->m_sPartKey;		
		PlayFile(fullUri.c_str());
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
cPlayMenu::cPlayMenu(const char *title, plexclient::plexgdm* gdm, int c0, int c1, int c2, int c3, int c4)
:cOsdMenu(title, c0, c1, c2, c3, c4)
{
	SetHasHotkeys();
	
	plexclient::PlexServer *server = gdm->GetPServer();
	
	if(server) {
		std::string serverName = server->GetServerName();
		Add(new cOsdItem(hk(serverName.c_str()), osUser1));
	}
	else {
		Add(new cOsdItem("No Plex Media Server found."));
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

    switch (state) {
	case osUser1:
	    ShowBrowser = 1;
	    return osPlugin;		// restart with OSD browser
	default:
	    break;
    }
    return state;
}

//////////////////////////////////////////////////////////////////////////////
//	cOsd
//////////////////////////////////////////////////////////////////////////////



volatile char cMyOsd::Dirty;		///< flag force redraw everything

/**
**	Sets this OSD to be the active one.
**
**	@param on	true on, false off
**
**	@note only needed as workaround for text2skin plugin with
**	undrawn areas.
*/
void cMyOsd::SetActive(bool on)
{
    dsyslog("[plex]%s: %d\n", __FUNCTION__, on);

    if (Active() == on) {
	return;				// already active, no action
    }
    cOsd::SetActive(on);

    // ignore sub-title, if menu is open
    if (OsdLevel >= OSD_LEVEL_SUBTITLES && IsOpen()) {
	return;
    }

    if (on) {
	Dirty = 1;
	// only flush here if there are already bitmaps
	//if (GetBitmap(0)) {
	//    Flush();
	//}
	OsdOpen();
    } else {
	OsdClose();
    }
}

/**
**	Constructor OSD.
**
**	Initializes the OSD with the given coordinates.
**
**	@param left	x-coordinate of osd on display
**	@param top	y-coordinate of osd on display
**	@param level	level of the osd (smallest is shown)
*/
cMyOsd::cMyOsd(int left, int top, uint level)
:cOsd(left, top, level)
{
    /* FIXME: OsdWidth/OsdHeight not correct!
       dsyslog("[plex]%s: %dx%d+%d+%d, %d\n", __FUNCTION__, OsdWidth(),
       OsdHeight(), left, top, level);
     */

    OsdLevel = level;
    SetActive(true);
}

/**
**	OSD Destructor.
**
**	Shuts down the OSD.
*/
cMyOsd::~cMyOsd(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
    SetActive(false);
    // done by SetActive: OsdClose();
}

/**
**	Actually commits all data to the OSD hardware.
*/
void cMyOsd::Flush(void)
{
    cPixmapMemory *pm;

    dsyslog("[plex]%s: level %d active %d\n", __FUNCTION__, OsdLevel,
	Active());

    if (!Active()) {			// this osd is not active
	return;
    }
    // don't draw sub-title if menu is active
    if (OsdLevel >= OSD_LEVEL_SUBTITLES && IsOpen()) {
	return;
    }
    //
    //	VDR draws subtitle without clearing the old
    //
    if (OsdLevel >= OSD_LEVEL_SUBTITLES) {
	OsdClear();
	cMyOsd::Dirty = 1;
	dsyslog("[plex]%s: subtitle clear\n", __FUNCTION__);
    }

    if (!IsTrueColor()) {
	cBitmap *bitmap;
	int i;

	// draw all bitmaps
	for (i = 0; (bitmap = GetBitmap(i)); ++i) {
	    uint8_t *argb;
	    int x;
	    int y;
	    int w;
	    int h;
	    int x1;
	    int y1;
	    int x2;
	    int y2;

	    // get dirty bounding box
	    if (Dirty) {		// forced complete update
		x1 = 0;
		y1 = 0;
		x2 = bitmap->Width() - 1;
		y2 = bitmap->Height() - 1;
	    } else if (!bitmap->Dirty(x1, y1, x2, y2)) {
		continue;		// nothing dirty continue
	    }
	    // convert and upload only dirty areas
	    w = x2 - x1 + 1;
	    h = y2 - y1 + 1;
	    if (1) {			// just for the case it makes trouble
		int width;
		int height;
		double video_aspect;

		::GetOsdSize(&width, &height, &video_aspect);
		if (w > width) {
		    w = width;
		    x2 = x1 + width - 1;
		}
		if (h > height) {
		    h = height;
		    y2 = y1 + height - 1;
		}
	    }
#ifdef DEBUG
	    if (w > bitmap->Width() || h > bitmap->Height()) {
		esyslog(tr("[plex]: dirty area too big\n"));
		abort();
	    }
#endif
	    argb = (uint8_t *) malloc(w * h * sizeof(uint32_t));
	    for (y = y1; y <= y2; ++y) {
		for (x = x1; x <= x2; ++x) {
		    ((uint32_t *) argb)[x - x1 + (y - y1) * w] =
			bitmap->GetColor(x, y);
		}
	    }
	    dsyslog("[plex]%s: draw %dx%d%+d%+d bm\n", __FUNCTION__, w, h,
		Left() + bitmap->X0() + x1, Top() + bitmap->Y0() + y1);
	    OsdDrawARGB(Left() + bitmap->X0() + x1, Top() + bitmap->Y0() + y1,
		w, h, argb);

	    bitmap->Clean();
	    // FIXME: reuse argb
	    free(argb);
	}
	cMyOsd::Dirty = 0;
	return;
    }

    LOCK_PIXMAPS;
    while ((pm = RenderPixmaps())) {
	int x;
	int y;
	int w;
	int h;

	x = Left() + pm->ViewPort().X();
	y = Top() + pm->ViewPort().Y();
	w = pm->ViewPort().Width();
	h = pm->ViewPort().Height();

	dsyslog("[plex]%s: draw %dx%d%+d%+d %p\n", __FUNCTION__, w, h, x, y,
	    pm->Data());
	OsdDrawARGB(x, y, w, h, pm->Data());

	delete pm;
    }
    cMyOsd::Dirty = 0;
}

//////////////////////////////////////////////////////////////////////////////
//	cOsdProvider
//////////////////////////////////////////////////////////////////////////////

cOsd *cMyOsdProvider::Osd;		///< single osd

/**
**	Create a new OSD.
**
**	@param left	x-coordinate of OSD
**	@param top	y-coordinate of OSD
**	@param level	layer level of OSD
*/
cOsd *cMyOsdProvider::CreateOsd(int left, int top, uint level)
{
    dsyslog("[plex]%s: %d, %d, %d\n", __FUNCTION__, left, top, level);

    return Osd = new cMyOsd(left, top, level);
}

/**
**	Check if this OSD provider is able to handle a true color OSD.
**
**	@returns true we are able to handle a true color OSD.
*/
bool cMyOsdProvider::ProvidesTrueColor(void)
{
    return true;
}

/**
**	Create cOsdProvider class.
*/
cMyOsdProvider::cMyOsdProvider(void)
:  cOsdProvider()
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
}


//////////////////////////////////////////////////////////////////////////////
//	cDevice
//////////////////////////////////////////////////////////////////////////////

/**
**	Device constructor.
*/
cMyDevice::cMyDevice(void)
{
    dsyslog("[plex]%s\n", __FUNCTION__);
}

/**
**	Device destructor. (never called!)
*/
cMyDevice::~cMyDevice(void)
{
    dsyslog("[plex]%s:\n", __FUNCTION__);
}

/**
**	Informs a device that it will be the primary device.
**
**	@param on	flag if becoming or loosing primary
*/
void cMyDevice::MakePrimaryDevice(bool on)
{
    dsyslog("[plex]%s: %d\n", __FUNCTION__, on);

    cDevice::MakePrimaryDevice(on);
    if (on) {
	new cMyOsdProvider();
    }
}

/**
**	Returns the width, height and pixel_aspect ratio the OSD.
**
**	FIXME: Called every second, for nothing (no OSD displayed)?
*/
void cMyDevice::GetOsdSize(int &width, int &height, double &pixel_aspect)
{
    if (!&width || !&height || !&pixel_aspect) {
	esyslog(tr("[plex]: GetOsdSize invalid pointer(s)\n"));
	return;
    }
    ::GetOsdSize(&width, &height, &pixel_aspect);
}

//////////////////////////////////////////////////////////////////////////////
//	cPlugin
//////////////////////////////////////////////////////////////////////////////

static cMyDevice *MyDevice;		///< dummy device needed for osd

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
	pPlexgdm->stopRegistration();
	plexclient::ControlServer::GetInstance().Stop();
	delete(pPlexgdm);
	delete(pService);
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
    return::CommandLineHelp();
}

/**
**	Process the command line arguments.
*/
bool cMyPlugin::ProcessArgs(int argc, char *argv[])
{
    return::ProcessArgs(argc, argv);
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
	
	pPlexgdm = new plexclient::plexgdm();
	pPlexgdm->discover();	
	plexclient::PlexServer *pServer = pPlexgdm->GetPServer();
	if (pServer != 0) {
		pPlexgdm->clientDetails(Config::GetInstance().GetUUID(), DESCRIPTION, "3200", "VDR", VERSION);
		pPlexgdm->Start();
		
		pService = new plexclient::Plexservice(pServer);
		pService->GetMyPlexToken();
		pService->Authenticate();
		
		plexclient::ControlServer::GetInstance().Start();
		
	} else {
		perror("No Plexserver found");
	}
	
    MyDevice = new cMyDevice;

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
		return new cPlexBrowser("Newest", this->pService);
    }
    return new cPlayMenu("Plex", pPlexgdm);
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
    if (strcmp(id, PLEX_OSD_3DMODE_SERVICE) == 0) {
	VideoSetOsd3DMode(0);
	Play_Osd3DModeService_v1_0_t *r =
	    (Play_Osd3DModeService_v1_0_t *) data;
	VideoSetOsd3DMode(r->Mode);
	return true;
    }
    return false;
}

/**
**	Return SVDRP commands help pages.
**
**	return a pointer to a list of help strings for all of the plugin's
**	SVDRP commands.
*/
const char **cMyPlugin::SVDRPHelpPages(void)
{
    static const char *HelpPages[] = {
	"3DOF\n" "	  TURN OFF 3D", "3DTB\n" "    TURN ON 3D TB",
	"3DSB\n" "	  TURN ON 3D SBS", NULL
    };
    return HelpPages;
}

/**
**	Handle SVDRP commands.
**
**	@param command		SVDRP command
**	@param option		all command arguments
**	@param reply_code	reply code
*/
cString cMyPlugin::SVDRPCommand(const char *command,
    __attribute__ ((unused)) const char *option,
    __attribute__ ((unused)) int &reply_code)
{
    if (!strcasecmp(command, "3DOF")) {
	VideoSetOsd3DMode(0);
	return "3d off";
    }
    if (!strcasecmp(command, "3DSB")) {
	VideoSetOsd3DMode(1);
	return "3d sbs";
    }
    if (!strcasecmp(command, "3DTB")) {
	VideoSetOsd3DMode(2);
	return "3d tb";
    }
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

//////////////////////////////////////////////////////////////////////////////

int OldPrimaryDevice;			///< old primary device

/**
**	Enable dummy device.
*/
extern "C" void EnableDummyDevice(void)
{
    OldPrimaryDevice = cDevice::PrimaryDevice()->DeviceNumber() + 1;

    dsyslog("[plex]: primary device %d to new %d\n", OldPrimaryDevice,
	MyDevice->DeviceNumber() + 1);

    //if (!cDevice::SetPrimaryDevice(MyDevice->DeviceNumber() + 1)) {
    DoMakePrimary = MyDevice->DeviceNumber() + 1;
    cOsdProvider::Shutdown();
    //}
}

/**
**	Disable dummy device.
*/
extern "C" void DisableDummyDevice(void)
{
    dsyslog("[plex]: primary device %d to old %d\n",
	cDevice::PrimaryDevice()->DeviceNumber() + 1, OldPrimaryDevice);

    //if (!cDevice::SetPrimaryDevice(OldPrimaryDevice)) {
    DoMakePrimary = OldPrimaryDevice;
    OldPrimaryDevice = 0;
    cOsdProvider::Shutdown();
    //}
}

VDRPLUGINCREATOR(cMyPlugin);		// Don't touch this!
