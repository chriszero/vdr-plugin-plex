#include "Config.h"


Config::Config() {
	s_username = "username";
	s_password = "password";
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
	
	Add(new cMenuEditBoolItem(tr("Hide main menu entry"), (int*)&Config::GetInstance().HideMainMenuEntry, trVDR("no"), trVDR("yes")));
    Add(new cMenuEditBoolItem(tr("Disable remote"), (int*)&Config::GetInstance().DisableRemote, trVDR("no"), trVDR("yes")));
	Add(new cMenuEditStrItem(tr("Plex Username"), Username, STRING_SIZE));
	Add(new cMenuEditStrItem(tr("Plex Password"), Password, STRING_SIZE));
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
	
    SetupStore("HideMainMenuEntry", Config::GetInstance().HideMainMenuEntry);
    SetupStore("DisableRemote", Config::GetInstance().DisableRemote);
	SetupStore("Username", Config::GetInstance().s_username.c_str());
	SetupStore("Password", Config::GetInstance().s_password.c_str());
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());
}