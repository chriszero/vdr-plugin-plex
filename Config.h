#ifndef CONFIG_H
#define CONFIG_H

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//VDR
#include <vdr/osd.h>
#include <vdr/menuitems.h>



/// vdr-plugin version number.
/// Makefile extracts the version number for generating the file name
/// for the distribution archive.
static const char *const VERSION = "0.0.1"
#ifdef GIT_REV
    "-GIT" GIT_REV
#endif
    ;

#define STRING_SIZE 256

class Config
{
	
public:
	static Config& GetInstance() {
		static Config instance;
		return instance;
	}

	std::string s_username = "username";
	std::string s_password = "password";
	
	bool HideMainMenuEntry;
	bool DisableRemote;
	
	std::string GetUUID();
	void SetUUID(const char* uuid);
	std::string GetHostname();
	std::string GetLanguage();
	std::string GetUsername();
	std::string GetPassword();
	

private:
	Config() {}
	std::string s_uuid;
	std::string s_hostname;
};


//////////////////////////////////////////////////////////////////////////////
//	cMenuSetupPage
//////////////////////////////////////////////////////////////////////////////

/**
**	Play plugin menu setup page class.
*/
class cMyMenuSetupPage:public cMenuSetupPage
{
  protected:
	char Username[STRING_SIZE];
	char Password[STRING_SIZE];
	char Uuid[STRING_SIZE];

    virtual void Store(void);

  public:
     cMyMenuSetupPage(void);
    virtual eOSState ProcessKey(eKeys);	// handle input
};

#endif // CONFIG_H
