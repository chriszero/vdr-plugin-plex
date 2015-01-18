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

#ifdef CRYPTOPP
#include <cryptopp/cryptlib.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#endif

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
	static std::string GetUniversalTranscodeUrl(Video* video, int offset = 0, PlexServer* server = 0);

	static MediaContainer* GetMediaContainer(std::string fullUrl);

protected:
	static std::string encode(std::string message);

private:
	Poco::Mutex m_mutex;
	// Never Access m_sToken directly! => possible race condition
	std::string m_sToken;

	std::string USERAGENT;

	Poco::Net::HTTPClientSession *m_pPlexSession;
	PlexServer *pServer;

	Poco::Net::HTTPClientSession* GetHttpSession(bool createNew = false);
	Poco::Net::HTTPRequest* CreateRequest(std::string path);

#ifdef CRYPTOPP
protected:
	std::string computeHMAC(std::string message);
public:
	std::string GetTranscodeUrl(Video* video);
#endif

};

}

#endif // PLEXSERVICE_H
