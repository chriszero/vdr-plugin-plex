#ifndef CONFIG_H
#define CONFIG_H

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//VDR
#include <vdr/osd.h>
#include <vdr/menuitems.h>

#define STRING_SIZE 256

struct ViewEntry {
    std::string Name;
    std::string PlexPath;
};

enum ViewMode {
    Cover = 0,
    List = 1,
    Detail = 2
};

class Config {

public:
    static Config &GetInstance() {
        static Config instance;
        return instance;
    }

    static const char *viewModeNames[];

    std::string s_username;
    std::string s_password;
    std::string s_serverHost;
    int ServerPort;

    bool HideMainMenuEntry;
    bool UseCustomTranscodeProfile;
    bool UsePlexAccount;
    bool UseConfiguredServer;
    bool UseAc3;
    int BufferSize;

    int CoverGridColumns;
    int CoverGridRows;

    int ListGridColumns;
    int ListGridRows;

    int DetailGridColumns;
    int DetailGridRows;

    int ExtrasGridColumns;
    int ExtrasGridRows;

    ViewMode DefaultViewMode;

    std::vector<ViewEntry> m_viewentries;
    std::vector<ViewEntry> m_serverViewentries;

    bool ScrollByPage;
    bool ScrollAllAround;
    bool UseMpv;

    std::string GetUUID();

    void SetUUID(const char *uuid);

    std::string GetHostname();

    std::string GetLanguage();

    std::string GetUsername();

    std::string GetPassword();

    //int ThumbHeight() { return 1080 / CoverGridRows; };
    //int ThumbWidth() { return 1920 / CoverGridColumns; };
    //int ArtHeight() { return 1080 / 4; };
    //int ArtWidth() { return 1920 / 4; };
    int ThumbHeight() { return 1080; };

    int ThumbWidth() { return 1920; };

    int ArtHeight() { return 1080; };

    int ArtWidth() { return 1920; };

    int BannerHeight() { return 1080 / 2; };

    int BannerWidth() { return 1920 / 2; };

    bool Parse(const char *name, const char *value);


private:
    Config();

    std::string s_uuid;
    std::string s_hostname;
};


//////////////////////////////////////////////////////////////////////////////
//	cMenuSetupPage
//////////////////////////////////////////////////////////////////////////////

/**
**	Play plugin menu setup page class.
*/
class cMyMenuSetupPage : public cMenuSetupPage {
protected:
    char Username[STRING_SIZE];
    char Password[STRING_SIZE];
    char Uuid[STRING_SIZE];
    char ServerHost[STRING_SIZE];
    int ServerPort;
    int UseConfiguredServer;
    int HideMainMenuEntry;
    int UseCustomTranscodeProfile;
    int UsePlexAccount;
    int CoverGridColumns;
    int CoverGridRows;
    int DetailGridColumns;
    int DetailGridRows;
    int ListGridColumns;
    int ListGridRows;
    int ExtrasGridColumns;
    int ExtrasGridRows;
    int DefaultViewMode;
    int UseMpv;
    int ScrollByPage;
    int ScrollAllAround;
    int UseAc3;
    int BufferSize;

    virtual void Store(void);

public:
    cMyMenuSetupPage(void);

    virtual eOSState ProcessKey(eKeys);    // handle input
};

#endif // CONFIG_H
