#include "Plexservice.h"

#include "PlexHelper.h"
#include "plexgdm.h"
#include <memory>

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
		
		Poco::Net::HTTPClientSession session(pServer->GetIpAdress(), pServer->GetPort());
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

		Poco::Net::HTTPClientSession session(pServer->GetIpAdress(), pServer->GetPort());
		session.sendRequest(*pRequest);
		Poco::Net::HTTPResponse response;
		std::istream &rs = session.receiveResponse(response);

		dsyslog("[plex] URI: http://%s:%d%s", pServer->GetIpAdress().c_str(), pServer->GetPort(), uri.c_str());

		std::shared_ptr<MediaContainer> pAllsections(new MediaContainer(&rs, pServer));
		
		session.abort();
		return pAllsections;

	} catch (Poco::Exception &exc) {
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
		if(!token.empty())
			pRequest->add("X-Plex-Token", token);
	}

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

	MediaContainer allsections(&rs, plexgdm::GetInstance().GetServer(fileuri.getHost(), fileuri.getPort()));
	
	session.abort();
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
	PlexServer* pSrv = server ? server : video->m_pServer;
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
	//params << "&subtitles=burn";
	//params << "&subtitleSize=90";
	//params << "&skipSubtitles=1";
	//params << "&audioBoost=100";
	params << "&videoResolution=1920x1080";
	params << "&videoQuality=100";
	params << "&session=" << encode(Config::GetInstance().GetUUID()); // TODO: generate Random SessionID


	return pSrv->GetUri() + params.str();
}

}
