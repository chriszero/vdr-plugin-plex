#include "Plexservice.h"
#include <memory>

namespace plexclient
{

Plexservice::Plexservice(PlexServer *server)
{
	pServer = server;
	USERAGENT = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17";
}

Plexservice::~Plexservice()
{
}

Poco::Net::HTTPClientSession* Plexservice::GetHttpSession(bool createNew)
{
	try {
		if(pServer == 0) {
			return NULL;
		}
		if (m_pPlexSession == 0 || createNew) {
			if (createNew) {
				//delete m_pPlexSession;
			}
			m_pPlexSession = new Poco::Net::HTTPClientSession(pServer->GetIpAdress(), pServer->GetPort());
			m_pPlexSession->setKeepAlive(true);
		}
	}
	catch(Poco::Exception &exc) {
		esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
		m_pPlexSession = 0;
	}
	return m_pPlexSession;
}

std::string Plexservice::GetMyPlexToken()
{
	// Syncronize
	Poco::Mutex::ScopedLock lock(m_mutex);
	
	//todo: cache token in file or db
	if(&m_sToken != 0 || m_sToken.empty()) {
		std::stringstream ss;
		Poco::Base64Encoder b64(ss);

		b64 << Config::GetInstance().GetUsername() << ":" << Config::GetInstance().GetPassword();

		b64.close();
		m_sToken = ss.str();

		Poco::Net::Context::Ptr context = new Poco::Net::Context(
		    Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_NONE, // VERIFY_NONE...?!
		    9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

		try {
			Poco::Net::HTTPSClientSession plexSession("plex.tv", 443, context);
			Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/users/sign_in.xml", Poco::Net::HTTPMessage::HTTP_1_1);

			request.add("X-Plex-Platform", "VDR");
			request.add("X-Plex-Platform-Version", "2.0.4");
			request.add("X-Plex-Provides", "player");
			request.add("X-Plex-Product", "plex for vdr");
			request.add("X-Plex-Version", "0.0.1a");
			request.add("X-Plex-Device", "Linux");
			request.add("X-Plex-Client-Identifier", "plex for vdr");
			request.add("Authorization", Poco::format("Basic %s", m_sToken));

			plexSession.sendRequest(request);

			Poco::Net::HTTPResponse response;
			std::istream &rs = plexSession.receiveResponse(response);
			// parse the XML Response
			user *u = new user(&rs);
			m_sToken = u->authenticationToken;
			plexSession.detachSocket();
		} catch (Poco::Exception &exc) {
			esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
		}

	}
	return m_sToken;
}

void Plexservice::Authenticate()
{
	try {
		if(GetHttpSession(true)) {
			std::string token = GetMyPlexToken();
			Poco::Net::HTTPRequest *pRequest = CreateRequest("/?X-Plex-Token=" + token);

			m_pPlexSession->sendRequest(*pRequest);
			Poco::Net::HTTPResponse response;
			/*std::istream &rs = */
			m_pPlexSession->receiveResponse(response);

			// TODO: process response
			//Poco::StreamCopier::copyStream(rs, std::cout);
			delete pRequest;
		}
		else {
			esyslog("[plex] %s HttpSession is NULL", __func__);
		}
	} catch (Poco::Exception &exc) {
		esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
	}
}
/*
void Plexservice::DiscoverAllServers()
{
	// try automatic discovery via multicast
	plexgdm gdmClient = plexgdm();
	gdmClient.discover();
	pServer = gdmClient.GetPServer();
	if (pServer == 0) {
		perror("No Plexserver found");
	}
}
 */

PlexServer* Plexservice::GetServer() {
	return pServer;
}

MediaContainer* Plexservice::GetAllSections()
{
	return GetSection("");
}

MediaContainer* Plexservice::GetSection(std::string section)
{
	std::string token = GetMyPlexToken();
	if(GetHttpSession(true)) {

		Poco::Net::HTTPRequest *pRequest;
		if(section[0]=='/') { // Full URI?
			pRequest = CreateRequest(Poco::format("%s?X-Plex-Token=%s", section, token));
		} else if(false == section.empty()) {
			pRequest = CreateRequest(Poco::format("/library/sections/%s?X-Plex-Token=%s", section, token));
		} else {
			pRequest = CreateRequest("/library/sections/?X-Plex-Token=" + token);
		}

		m_pPlexSession->sendRequest(*pRequest);
		Poco::Net::HTTPResponse response;
		std::istream &rs = m_pPlexSession->receiveResponse(response);

		dsyslog("[plex] URI: %s[s%]", m_pPlexSession->getHost().c_str(), pRequest->getURI().c_str());
		
		MediaContainer* pAllsections = new MediaContainer(&rs);
		//Poco::StreamCopier::copyStream(rs, std::cout);
		delete pRequest;
		return pAllsections;
	
	}
	else {
		esyslog("[plex] %s HttpSession is NULL", __func__);
		return 0;
	}
}

Poco::Net::HTTPRequest* Plexservice::CreateRequest(std::string path)
{
	Poco::Net::HTTPRequest *pRequest = new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_GET,
	        path, Poco::Net::HTTPMessage::HTTP_1_1);

	pRequest->add("User-Agent", USERAGENT);

	pRequest->add("X-Plex-Client-Capabilities", "protocols=shoutcast,http-video;videoDecoders=h264{profile:high&resolution:1080&level:51};audioDecoders=mp3,aac");
	pRequest->add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
	pRequest->add("X-Plex-Device", "PC");
	pRequest->add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
	pRequest->add("X-Plex-Language", Config::GetInstance().GetLanguage());
	pRequest->add("X-Plex-Model", "Linux");
	pRequest->add("X-Plex-Platform", "VDR");
	pRequest->add("X-Plex-Product", "plex for vdr");
	pRequest->add("X-Plex-Provides", "player");
	pRequest->add("X-Plex-Version", "0.0.1a");

	return pRequest;
}

MediaContainer* Plexservice::GetMediaContainer(std::string fullUrl) {
	
	Poco::URI fileuri(fullUrl);
	
	Poco::Net::HTTPRequest* pRequest = new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_GET,
	        fileuri.getPath(), Poco::Net::HTTPMessage::HTTP_1_1);

	pRequest->add("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.52 Safari/537.17");

	pRequest->add("X-Plex-Client-Capabilities", "protocols=shoutcast,http-video;videoDecoders=h264{profile:high&resolution:1080&level:51};audioDecoders=mp3,aac");
	pRequest->add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
	pRequest->add("X-Plex-Device", "PC");
	pRequest->add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
	pRequest->add("X-Plex-Language", Config::GetInstance().GetLanguage());
	pRequest->add("X-Plex-Model", "Linux");
	pRequest->add("X-Plex-Platform", "VDR");
	pRequest->add("X-Plex-Product", "plex for vdr");
	pRequest->add("X-Plex-Provides", "player");
	pRequest->add("X-Plex-Version", "0.0.1a");
	
	Poco::Net::HTTPClientSession* session = new Poco::Net::HTTPClientSession(fileuri.getHost(), fileuri.getPort());
	
	session->sendRequest(*pRequest);
	Poco::Net::HTTPResponse response;
	std::istream &rs = session->receiveResponse(response);

	std::cout << "URI: " << session->getHost() << "[" << pRequest->getURI() << "]" << std::endl;
	
	delete pRequest;
	delete session;
	
	MediaContainer* pAllsections = new MediaContainer(&rs);
	//Poco::StreamCopier::copyStream(rs, std::cout);
	return pAllsections;
}

}

