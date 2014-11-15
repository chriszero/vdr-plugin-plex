#ifndef PLEXSERVICE_H
#define PLEXSERVICE_H

//#include "plexgdm.h"
#include "PlexServer.h"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

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
	MediaContainer* GetAllSections();
	MediaContainer* GetSection(std::string section);
	void GetAuthDetails();
	std::string GetMyPlexToken();
	void Authenticate();
	//void DiscoverFirstServer();
	PlexServer* GetServer();
	
	static MediaContainer* GetMediaContainer(std::string fullUrl);

private:
	const std::string USERAGENT = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17";

	Poco::Net::HTTPClientSession *m_pPlexSession = 0;
	PlexServer *pServer = 0;
	std::string m_sToken;

	Poco::Net::HTTPClientSession* GetHttpSession(bool createNew = false);
	Poco::Net::HTTPRequest* CreateRequest(std::string path);

};

}

#endif // PLEXSERVICE_H
