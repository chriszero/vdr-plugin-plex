#ifndef PLEXSERVICE_H
#define PLEXSERVICE_H

//#include "plexgdm.h"
#include "PlexServer.h"

#include <sstream>
#include <iostream>
#include <string>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <time.h>

#include <Poco/Base64Encoder.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/Context.h>
#include <Poco/Exception.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/Format.h>

#include <Poco/StreamCopier.h>

#include "Config.h"
#include "user.h"
#include "MediaContainer.h"

namespace plexclient
{

class Plexservice
{
public:
	Plexservice(PlexServer *server);
	~Plexservice();

	void DisplaySections();
	std::shared_ptr<MediaContainer> GetSection(std::string section, bool putOnStack = true);
	std::shared_ptr<MediaContainer> GetLastSection();
	void GetAuthDetails();
	void Authenticate();
	
	PlexServer* GetServer();
	static std::string GetUniversalTranscodeUrl(Video* video, int offset = 0, PlexServer* server = 0);
	static std::string GetMyPlexToken();
	static MediaContainer GetMediaContainer(std::string fullUrl);
	static std::string encode(std::string message);

private:
	Poco::Mutex m_mutex;
	Poco::Net::HTTPClientSession *m_pPlexSession;
	PlexServer *pServer;

	std::stack<std::string> m_vUriStack;

	Poco::Net::HTTPClientSession* GetHttpSession(bool createNew = false);
	Poco::Net::HTTPRequest* CreateRequest(std::string path);
};

}

#endif // PLEXSERVICE_H
