#include "Plexservice.h"

#include "PlexHelper.h"
#include "plexgdm.h"
#include <memory>
#include <Poco/Net/NetException.h>

namespace plexclient
{

Plexservice::Plexservice(PlexServer *server)
{
	pServer = server;
}

Plexservice::Plexservice(PlexServer *server, std::string startUri)
{
	pServer = server;
	StartUri = startUri;
}

std::string Plexservice::GetMyPlexToken()
{
	static bool done;
	static std::string myToken;

	//todo: cache token in file or db
	if(!done) {
		std::stringstream ss;
		Poco::Base64Encoder b64(ss);

		b64 << Config::GetInstance().GetUsername() << ":" << Config::GetInstance().GetPassword();

		b64.close();
		std::string tempToken = ss.str();

		Poco::Net::Context::Ptr context = new Poco::Net::Context(
		    Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_NONE, // VERIFY_NONE...?!
		    9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

		try {
			Poco::Net::HTTPSClientSession plexSession("plex.tv", 443, context);
			Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/users/sign_in.xml", Poco::Net::HTTPMessage::HTTP_1_1);

			PlexHelper::AddHttpHeader(request);
			request.add("Authorization", Poco::format("Basic %s", tempToken));

			plexSession.sendRequest(request);

			Poco::Net::HTTPResponse response;
			std::istream &rs = plexSession.receiveResponse(response);
			if(response.getStatus() == 201) {
				// parse the XML Response
				user u(&rs);
				myToken = u.authenticationToken;
				isyslog("[plex] plex.tv login successfull.");
			} else {
				esyslog("[plex] plex.tv Login failed, check you creditials.");
			}
			done = true;
			plexSession.abort();
		} catch (Poco::Exception &exc) {
			esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
			done = true;
		}

	}
	return myToken;
}

void Plexservice::Authenticate()
{
	if(!Config::GetInstance().UsePlexAccount) return;
	try {
		std::string token = GetMyPlexToken();
		auto pRequest = CreateRequest("/?X-Plex-Token=" + token);

		Poco::Net::HTTPClientSession session(pServer->GetHost(), pServer->GetPort());
		session.sendRequest(*pRequest);
		Poco::Net::HTTPResponse response;
		/*std::istream &rs = */
		session.receiveResponse(response);
		session.abort();
		// TODO: process response
		//Poco::StreamCopier::copyStream(rs, std::cout);
	} catch (Poco::Exception &exc) {
		esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
	}
}

void Plexservice::UpdateResources()
{
	// We must be autenticated
	// https://plex.tv/api/resources?includeHttps=1
	if(!Config::GetInstance().UsePlexAccount) {
		isyslog("[plex] To access remote servers, please login with your plex.tv account.");
		return; // Plugin is used without plex.tv login
	}
	isyslog("[plex] Updating remote resources...");

	std::shared_ptr<MediaContainer> pContainer = GetMediaContainer("https://plex.tv/api/resources?includeHttps=1");

	for(std::vector<Device>::iterator d_it = pContainer->m_vDevices.begin(); d_it != pContainer->m_vDevices.end(); ++d_it) {

		// check device is a server
		if(d_it->m_sProvides.find("server") != std::string::npos) {
			// is it a remote server?
			if(d_it->m_bPublicAddressMatches == false) {
				// pick remote connection
				for(std::vector<Connection>::iterator c_it = d_it->m_vConnections.begin(); c_it != d_it->m_vConnections.end(); ++c_it) {
					if(c_it->m_bLocal == false) {
						dsyslog("[plex] Found remote server: %s", d_it->m_sName.c_str());
						// a remote Server
						plexgdm::GetInstance().AddServer(PlexServer(c_it->m_sUri, d_it->m_sName, d_it->m_sClientIdentifier, d_it->m_sAccessToken, d_it->m_bOwned, c_it->m_bLocal));
					}
				}
			}
		}
	}
}

PlexServer* Plexservice::GetServer()
{
	return pServer;
}

std::shared_ptr<MediaContainer> Plexservice::GetSection(std::string section, bool putOnStack)
{
	try {
		std::string uri;
		if(section[0]=='/') { // Full URI?
			uri = section;
		} else {
			uri = Poco::format("%s/%s", m_vUriStack.top(), section);
		}

		auto pRequest = CreateRequest(uri);
		if(putOnStack) {
			m_vUriStack.push(uri);
		}

		dsyslog("[plex] URI: %s%s", pServer->GetUri().c_str(), uri.c_str());
		//Poco::Net::HTTPClientSession session(pServer->GetHost(), pServer->GetPort());
		//session.sendRequest(*pRequest);
		pServer->GetClientSession()->sendRequest(*pRequest);
		Poco::Net::HTTPResponse response;
		std::istream &rs = pServer->GetClientSession()->receiveResponse(response);


		std::shared_ptr<MediaContainer> pAllsections(new MediaContainer(&rs, pServer));

		//session.abort();
		return pAllsections;

	} catch (Poco::Net::NetException &exc) {
		pServer->Offline = true;
		return 0;
	}
}

std::shared_ptr<MediaContainer> Plexservice::GetLastSection(bool current)
{
	if(m_vUriStack.size() > 1) {
		if(!current) {
			// discard last one
			m_vUriStack.pop();
		}
		std::string uri = m_vUriStack.top();
		return GetSection(uri, false);
	}
	return NULL;
}

bool Plexservice::IsRoot()
{
	return m_vUriStack.size() <= 1;
}

std::unique_ptr<Poco::Net::HTTPRequest> Plexservice::CreateRequest(std::string path)
{
	std::unique_ptr<Poco::Net::HTTPRequest> pRequest = std::unique_ptr<Poco::Net::HTTPRequest>(new Poco::Net::HTTPRequest(Poco::Net::HTTPRequest::HTTP_GET,
	        path, Poco::Net::HTTPMessage::HTTP_1_1));

	PlexHelper::AddHttpHeader(*pRequest);

	if(Config::GetInstance().UsePlexAccount) {
		// Add PlexToken to Header
		std::string token = GetMyPlexToken();
		if(pServer && !pServer->GetAuthToken().empty()) {
			pRequest->add("X-Plex-Token", pServer->GetAuthToken());
			dsyslog("[plex] Using server access token");
		} else if(!token.empty()) {
			pRequest->add("X-Plex-Token", token);
			dsyslog("[plex] Using global access token");
		}
	}

	return pRequest;
}

std::shared_ptr<MediaContainer> Plexservice::GetMediaContainer(std::string fullUrl)
{
	Poco::Net::HTTPClientSession* pSession = NULL;
	PlexServer* pServer = NULL;
	bool ownSession = false;
	try {
		Poco::URI fileuri(fullUrl);
		dsyslog("[plex] GetMediaContainer: %s", fullUrl.c_str());

		if(fileuri.getHost().find("plex.tv") != std::string::npos) {
			if(fileuri.getScheme().find("https") != std::string::npos) {
				pSession = new Poco::Net::HTTPSClientSession(fileuri.getHost(), fileuri.getPort());
			} else {
				pSession = new Poco::Net::HTTPClientSession(fileuri.getHost(), fileuri.getPort());
			}
			ownSession = true;
		}
		else {
			pServer = plexgdm::GetInstance().GetServer(fileuri.getHost(), fileuri.getPort());
			pSession = pServer->GetClientSession();
		}
		
		
		
		// > HTTPS

		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, fileuri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
		PlexHelper::AddHttpHeader(request);

		if(Config::GetInstance().UsePlexAccount) {
			// Add PlexToken to Header
			std::string token = GetMyPlexToken();
			if(pServer && !pServer->GetAuthToken().empty()) {
				request.add("X-Plex-Token", pServer->GetAuthToken());
				dsyslog("[plex] Using server access token");
			} else if(!token.empty()) {
				request.add("X-Plex-Token", token);
				dsyslog("[plex] Using global access token");
			}
		}


		pSession->sendRequest(request);
		Poco::Net::HTTPResponse response;
		std::istream &rs = pSession->receiveResponse(response);

		//Poco::StreamCopier::copyStream(rs, std::cout);

		std::shared_ptr<MediaContainer> pAllsections = std::shared_ptr<MediaContainer>(new MediaContainer(&rs, pServer));
		if (ownSession) delete pSession;
		return pAllsections;
	} catch (Poco::Net::NetException &exc) {
		std::cout << exc.displayText() << std::endl;
		return 0;
	}
}

std::string Plexservice::encode(std::string message)
{
	std::string temp;
	Poco::URI::encode(message, " !\"#$%&'()*+,/:;=?@[]", temp);
	return temp;
}

std::string Plexservice::GetUniversalTranscodeUrl(Video* video, int offset, PlexServer* server)
{
	PlexServer* pSrv = server ? server : video->m_pServer;
	std::stringstream params;
	params << "/video/:/transcode/universal/start.m3u8?";
	// Force set localhost and http
	Poco::URI pathUri(pSrv->GetUri()+video->m_sKey);
	pathUri.setHost("127.0.0.1");
	pathUri.setScheme("http");
	
	params << "path=" << encode(pathUri.toString());
	params << "&mediaIndex=0";
	params << "&partIndex=0";
	params << "&protocol=hls";
	params << "&offset=" << offset;
	params << "&fastSeek=0";
	params << "&directPlay=0";
	params << "&directStream=1";
	params << "&subtitles=burn";
	//params << "&subtitleSize=90";
	//params << "&skipSubtitles=1";
	//params << "&audioBoost=100";
	
	if(Config::GetInstance().UsePlexAccount) {
		if(!pSrv->GetAuthToken().empty()) {
			params << "&X-Plex-Token=" << pSrv->GetAuthToken();
		}
	}
	
	if(pSrv->IsLocal()) {
		params << "&videoResolution=1920x1080";
		params << "&maxVideoBitrate=20000";
		params << "&videoQuality=100";
	} else {
		params << "&videoResolution=1280x720";
		params << "&maxVideoBitrate=8000";
		params << "&videoQuality=100";
	}
	
	params << "&session=" << encode(Config::GetInstance().GetUUID()); // TODO: generate Random SessionID

	params << "&includeCodecs=1";
	params << "&copyts=1";

	if(Config::GetInstance().UseAc3) {
		params << "&X-Plex-Client-Profile-Extra=";
		if(Config::GetInstance().UseAc3)
			params << encode("add-transcode-target-audio-codec(type=videoProfile&context=streaming&protocol=hls&audioCodec=ac3)");

		//params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=lowerBound&name=video.height&value=1080)");
		//params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=lowerBound&name=video.frameRate&value=25)");
		//params << encode("+add-limitation(scope=videoCodec&scopeName=h264&type=upperBound&name=video.frameRate&value=25)");
	}
	return pSrv->GetUri() + params.str();
}

}
