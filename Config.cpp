#include "Config.h"


const char* Config::viewModeNames[]
{
	"Cover",
	"List",
	"Detail"
};

Config::Config() {
	s_username = "username";
	s_password = "password";
	CoverGridColumns = 7;
	CoverGridRows = 2;
	ListGridColumns = 1;
	ListGridRows = 12;
	DetailGridColumns = 1;
	DetailGridRows = 4;
	
	DefaultViewMode = ViewMode::Cover;
	
	ViewEntry en;
	en.Name = "Recently Added";
	en.PlexPath = "/library/recentlyAdded";
	m_viewentries.push_back(en);
	
	ViewEntry en2;
	en2.Name = "On Deck";
	en2.PlexPath = "/library/onDeck";
	m_viewentries.push_back(en2);
	
	ViewEntry en3;
	en3.Name = "Library";
	en3.PlexPath = "/library/sections";
	m_serverViewentries.push_back(en3);
	
	ViewEntry en4;
	en4.Name = "Video Channels";
	en4.PlexPath = "/video";
	m_serverViewentries.push_back(en4);
	
}

std::string Config::GetUUID() {
	if(s_uuid.empty()) {
		using Poco::UUIDGenerator;
		using Poco::UUID;
		UUIDGenerator &generator = UUIDGenerator::defaultGenerator();
		UUID uuid(generator.createRandom());
		s_uuid = uuid.toString();
	}
	return s_uuid;
}

void Config::SetUUID(const char* uuid) {
	if (uuid) s_uuid = std::string(uuid);
}


std::string Config::GetHostname() {
	if(s_hostname.empty()) {
		char hostname[1024];
		gethostname(hostname, 1024);
		s_hostname = std::string(hostname);
	}
	return s_hostname;
}

std::string Config::GetLanguage() {
	return "de";
}

std::string Config::GetUsername() {
	return s_username;
}

std::string Config::GetPassword() {
	return s_password;
}


bool Config::Parse(const char *name, const char *value)
{
	//dsyslog("[plex]%s: '%s' = '%s'\n", __FUNCTION__, name, value);
	bool parsed = true;
	if (strcasecmp(name, "HideMainMenuEntry") == 0) 	Config::GetInstance().HideMainMenuEntry = atoi(value) ? true : false;
	else if (strcasecmp(name, "UsePlexAccount") == 0) 	Config::GetInstance().UsePlexAccount = atoi(value) ? true : false;
	else if (strcasecmp(name, "UseCustomTranscodeProfile") == 0) 	Config::GetInstance().UseCustomTranscodeProfile = atoi(value) ? true : false;
	else if (strcasecmp(name, "Username") == 0) 		Config::GetInstance().s_username = std::string(value);
	else if (strcasecmp(name, "Password") == 0) 		Config::GetInstance().s_password = std::string(value);
	else if (strcasecmp(name, "UUID") == 0) 			Config::GetInstance().SetUUID(value);
	else if (strcasecmp(name, "UseConfiguredServer") == 0) 	Config::GetInstance().UseConfiguredServer = atoi(value) ? true : false;
	else if (strcasecmp(name, "ServerHost") == 0) 		Config::GetInstance().s_serverHost = std::string(value);
	else if (strcasecmp(name, "ServerPort") == 0) 	Config::GetInstance().ServerPort = atoi(value);
	else if (strcasecmp(name, "CoverGridColumns") == 0) 	Config::GetInstance().CoverGridColumns = atoi(value);
	else if (strcasecmp(name, "CoverGridRows") == 0) 	Config::GetInstance().CoverGridRows = atoi(value);
	else if (strcasecmp(name, "DetailGridColumns") == 0) 	Config::GetInstance().DetailGridColumns = atoi(value);
	else if (strcasecmp(name, "DetailGridRows") == 0) 	Config::GetInstance().DetailGridRows = atoi(value);
	else if (strcasecmp(name, "ListGridColumns") == 0) 	Config::GetInstance().ListGridColumns = atoi(value);
	else if (strcasecmp(name, "ListGridRows") == 0) 	Config::GetInstance().ListGridRows = atoi(value);
	else parsed = false;

	if(!parsed) {
		// Parse ViewEntries
		
	}
	
	return parsed;
}


//////////////////////////////////////////////////////////////////////////////
//	cMenuSetupPage
//////////////////////////////////////////////////////////////////////////////

/**
**	Process key for setup menu.
*/
eOSState cMyMenuSetupPage::ProcessKey(eKeys key)
{
	return cMenuSetupPage::ProcessKey(key);
}

/**
**	Constructor setup menu.
**
**	Import global config variables into setup.
*/
cMyMenuSetupPage::cMyMenuSetupPage(void)
{
	strn0cpy(Username, Config::GetInstance().s_username.c_str(), STRING_SIZE);
	strn0cpy(Password, Config::GetInstance().s_password.c_str(), STRING_SIZE);
	strn0cpy(Uuid, Config::GetInstance().GetUUID().c_str(), STRING_SIZE);
	strn0cpy(ServerHost, Config::GetInstance().s_serverHost.c_str(), STRING_SIZE);
	ServerPort = Config::GetInstance().ServerPort;
	UseConfiguredServer = Config::GetInstance().UseConfiguredServer;
	HideMainMenuEntry = Config::GetInstance().HideMainMenuEntry;
	UseCustomTranscodeProfile = Config::GetInstance().UseCustomTranscodeProfile;
	UsePlexAccount = Config::GetInstance().UsePlexAccount;
	CoverGridColumns = Config::GetInstance().CoverGridColumns;
	CoverGridRows = Config::GetInstance().CoverGridRows;
	DetailGridColumns = Config::GetInstance().DetailGridColumns;
	DetailGridRows = Config::GetInstance().DetailGridRows;
	ListGridColumns = Config::GetInstance().ListGridColumns;
	ListGridRows = Config::GetInstance().ListGridRows;
	
	DefaultViewMode = Config::GetInstance().DefaultViewMode;
	
	
	Add(new cMenuEditBoolItem(tr("Hide main menu entry"), (int*)&HideMainMenuEntry, trVDR("no"), trVDR("yes")));
	Add(new cMenuEditBoolItem(tr("Use custom transcoding profile"), (int*)&UseCustomTranscodeProfile, trVDR("no"), trVDR("yes")));
	Add(new cMenuEditBoolItem(tr("Use Plex account"), (int*)&UsePlexAccount, trVDR("no"), trVDR("yes")));
	Add(new cMenuEditStrItem(tr("Plex Username"), Username, STRING_SIZE));
	Add(new cMenuEditStrItem(tr("Plex Password"), Password, STRING_SIZE));
	
	Add(new cMenuEditBoolItem(tr("Use Custom Server"), (int*)&UseConfiguredServer, trVDR("no"), trVDR("yes")));
	Add(new cMenuEditStrItem(tr("Server Host"), ServerHost, STRING_SIZE));
	Add(new cMenuEditIntItem(tr("Server Port"), &ServerPort));
	
	Add(new cMenuEditStraItem(tr("Default View Mode"), &DefaultViewMode, 3, Config::viewModeNames));
	
	Add(new cMenuEditIntItem(tr("Cover Grid Columns"), &CoverGridColumns));
	Add(new cMenuEditIntItem(tr("Cover Grid Rows"), &CoverGridRows));
	
	Add(new cMenuEditIntItem(tr("Detail Grid Columns"), &DetailGridColumns));
	Add(new cMenuEditIntItem(tr("Detail Grid Rows"), &DetailGridRows));
	
	Add(new cMenuEditIntItem(tr("List Grid Columns"), &ListGridColumns));
	Add(new cMenuEditIntItem(tr("List Grid Rows"), &ListGridRows));
	
	cMenuEditStrItem* devUUID = new cMenuEditStrItem(tr("Current UUID"), Uuid, STRING_SIZE);
	devUUID->SetSelectable(false);
	Add(devUUID);
}

/**
**	Store setup.
*/
void cMyMenuSetupPage::Store(void)
{
	Config::GetInstance().s_username = std::string(Username);
	Config::GetInstance().s_password = std::string(Password);
	Config::GetInstance().HideMainMenuEntry = HideMainMenuEntry;
	Config::GetInstance().UseCustomTranscodeProfile = UseCustomTranscodeProfile;
	Config::GetInstance().UsePlexAccount = UsePlexAccount;
	Config::GetInstance().UseConfiguredServer = UseConfiguredServer;
	Config::GetInstance().s_serverHost = std::string(ServerHost);
	Config::GetInstance().ServerPort = ServerPort;
	Config::GetInstance().CoverGridColumns = CoverGridColumns;
	Config::GetInstance().CoverGridRows = CoverGridRows;
	Config::GetInstance().DetailGridColumns = DetailGridColumns;
	Config::GetInstance().DetailGridRows = DetailGridRows;
	Config::GetInstance().ListGridColumns = ListGridColumns;
	Config::GetInstance().ListGridRows = ListGridRows;
	Config::GetInstance().DefaultViewMode = (ViewMode)DefaultViewMode;
	
	SetupStore("UseCustomTranscodeProfile", Config::GetInstance().UseCustomTranscodeProfile);
	SetupStore("HideMainMenuEntry", Config::GetInstance().HideMainMenuEntry);
	SetupStore("UsePlexAccount", Config::GetInstance().UsePlexAccount);
	SetupStore("Username", Config::GetInstance().s_username.c_str());
	SetupStore("Password", Config::GetInstance().s_password.c_str());
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());
	SetupStore("UseConfiguredServer", Config::GetInstance().UseConfiguredServer);
	SetupStore("ServerHost", Config::GetInstance().s_serverHost.c_str());
	SetupStore("ServerPort", Config::GetInstance().ServerPort);
	SetupStore("CoverGridColumns", Config::GetInstance().CoverGridColumns);
	SetupStore("CoverGridRows", Config::GetInstance().CoverGridRows);
	SetupStore("DetailGridColumns", Config::GetInstance().DetailGridColumns);
	SetupStore("DetailGridRows", Config::GetInstance().DetailGridRows);
	SetupStore("ListGridColumns", Config::GetInstance().ListGridColumns);
	SetupStore("ListGridRows", Config::GetInstance().ListGridRows);
	SetupStore("DefaultViewMode", Config::GetInstance().DefaultViewMode);
}
