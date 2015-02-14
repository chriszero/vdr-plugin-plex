#include "Plexservice.h"

#include "PlexHelper.h"

namespace plexclient
{

Plexservice::Plexservice(PlexServer *server)
{
	pServer = server;
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
	} catch(Poco::Exception &exc) {
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

			PlexHelper::AddHttpHeader(request);
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
		} else {
			esyslog("[plex] %s HttpSession is NULL", __func__);
		}
	} catch (Poco::Exception &exc) {
		esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
	}
}

PlexServer* Plexservice::GetServer()
{
	return pServer;
}

MediaContainer* Plexservice::GetAllSections()
{
	return GetSection("");
}

MediaContainer* Plexservice::GetSection(std::string section)
{
	std::string token = GetMyPlexToken();
	if(GetHttpSession()) {

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

		MediaContainer* pAllsections = new MediaContainer(&rs, *pServer);
		//Poco::StreamCopier::copyStream(rs, std::cout);
		delete pRequest;
		return pAllsections;

	} else {
		esyslog("[plex] %s HttpSession is NULL", __func__);
		return 0;
	}
}

Poco::Net::HTTPRequest* Plexservice::CreateRequest(std::string path)
{
	Poco::Net::HTTPRequest *pRequest = new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_GET,
	        path, Poco::Net::HTTPMessage::HTTP_1_1);

	PlexHelper::AddHttpHeader(*pRequest);

	return pRequest;
}

MediaContainer Plexservice::GetMediaContainer(std::string fullUrl)
{
	Poco::URI fileuri(fullUrl);
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, fileuri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
	PlexHelper::AddHttpHeader(request);

	Poco::Net::HTTPClientSession session(fileuri.getHost(), fileuri.getPort());

	session.sendRequest(request);
	Poco::Net::HTTPResponse response;
	std::istream &rs = session.receiveResponse(response);

	MediaContainer allsections(&rs, PlexServer(fileuri.getHost(), fileuri.getPort()));

	return allsections;
}

std::string Plexservice::encode(std::string message)
{
	std::string temp;
	Poco::URI::encode(message, " !\"#$%&'()*+,/:;=?@[]", temp);
	return temp;
}


std::string Plexservice::GetUniversalTranscodeUrl(Video* video, int offset, PlexServer* server)
{
	PlexServer* pSrv = server ? server : &video->m_Server;
	std::stringstream params;
	params << "/video/:/transcode/universal/start.m3u8?";
	params << "path=" << encode(pSrv->GetUri() + video->m_sKey);
	params << "&mediaIndex=0";
	params << "&partIndex=0";
	params << "&protocol=hls";
	params << "&offset=" << offset;
	params << "&fastSeek=1";
	params << "&directPlay=0";
	params << "&directStream=1";
	params << "&maxVideoBitrate=20000";
	//params << "&subtitleSize=90";
	params << "&skipSubtitles=1";
	//params << "&audioBoost=100";
	params << "&videoResolution=1920x1080";
	params << "&videoQuality=100";
	params << "&session=" << encode(Config::GetInstance().GetUUID()); // TODO: generate Random SessionID


	return pSrv->GetUri() + params.str();
}

}
