#include "PlexHTTPRequestHandler.h"
#include <vdr/remote.h>
#include <unistd.h>

namespace plexclient
{

void PlexHTTPRequestHandler::AddHeaders(Poco::Net::HTTPServerResponse& response, Poco::Net::HTTPServerRequest& request) {
	if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
		response.setContentType("text/plain");	

		response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
		response.add("Connection", "Close");
		response.add("Access-Control-Max-Age", "1209600");
		response.add("Access-Control-Allow-Origin", "*");
		response.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT, HEAD");
		response.add("Access-Control-Allow-Headers", "x-plex-version, x-plex-platform-version, "
													 "x-plex-username, x-plex-client-identifier, "
													 "x-plex-target-client-identifier, x-plex-device-name, "
													 "x-plex-platform, x-plex-product, accept, x-plex-device");
	} else {
		response.setContentType("application/x-www-form-urlencoded");	
		
		response.add("Access-Control-Allow-Origin", "*");
		response.add("X-Plex-Version", VERSION);
		response.add("X-Plex-Client-Identifier", Config::GetInstance().GetUUID());
		response.add("X-Plex-Product", "player");
		response.add("X-Plex-Product", "PlexVDR");
		response.add("X-Plex-Device-Name", Config::GetInstance().GetHostname());
		response.add("X-Plex-Platform", "VDR");
		response.add("X-Plex-Model", "Linux");
		response.add("X-Plex-Device", "PC");
		response.add("X-Plex-Username", Config::GetInstance().GetUsername());
	}
	//header.MessageHeader(header);
}

std::map<std::string, std::string> PlexHTTPRequestHandler::ParseQuery(std::string query) {
	std::map<std::string, std::string> querymap;
	Poco::StringTokenizer queryTokens(query, "&");
	for(auto& token : queryTokens) {
		Poco::StringTokenizer subTokens(token, "=");
		querymap[subTokens[0]] = subTokens[1];
	}
	return querymap;
}

std::string PlexHTTPRequestHandler::GetOKMsg() {
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?> <Response code=\"200\" status=\"OK\" />";
}

void PlexHTTPRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){	
	AddHeaders(response, request);
	std::ostream& ostr = response.send();
	ostr << GetOKMsg();
}

void SubscribeRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
	// parse query
	Poco::URI uri(request.getURI());
	auto query = ParseQuery(uri.getQuery()); // port=32400&commandID=0&protocol=http
	std::string path = uri.getPath(); // /player/timeline/subscribe
	
	std::string uuid = request.get("X-Plex-Client-Identifier");
	int port = std::stoi(query["port"]);
	int command = std::stoi(query["commandID"]);
	SubscriptionManager::GetInstance().AddSubscriber(Subscriber(query["protocol"], request.getHost(), port, uuid, command));
	
	AddHeaders(response, request);
	
	std::ostream& ostr = response.send();
	ostr << GetOKMsg();
	response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
}

void ResourceRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
	AddHeaders(response, request);
	
	std::ostream& ostr = response.send();
	ostr << "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<MediaContainer>"
			"<Player title=\"" << Config::GetInstance().GetHostname() << "\""
			" protocol=\"plex\""
			" protocolVersion=\"1\""
			" protocolCapabilities=\"navigation,playback,timeline\""
			" machineIdentifier=\"" << Config::GetInstance().GetUUID() << "\""
			" product=\"PlexVDR\""
			" platform=\"Linux\""
			" platformVersion=\"" << VERSION << "\""
			" deviceClass=\"HTPC\""
			"/> </MediaContainer>";
	
	response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);

	
	std::cout << "Resources Response sent..." << std::endl;
}

void PlayerRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response){
	Poco::URI uri(request.getURI());
	//Poco::StringTokenizer pathTokens(uri, "/");
	auto query = ParseQuery(uri.getQuery());
	
	if(query.find("wait") != query.end() && std::stoi(query["wait"]) == 1) {
		usleep(900 * 1000);
	}
	
	if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS)  {
		AddHeaders(response, request);
		//response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
		std::ostream& ostr = response.send(); // Stream must not be empty!
		ostr << " ";
		response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
		std::cout << "OPTION Reply send" << std::endl;
		return;
	} 
	
	if(request.getURI().find("/poll")!= std::string::npos) {
		response.add("X-Plex-Client-Identifier",Config::GetInstance().GetUUID());
		response.add("Access-Control-Expose-Headers","X-Plex-Client-Identifier");
		response.add("Access-Control-Allow-Origin","*");
		response.setContentType("text/xml");
		
		std::ostream& ostr = response.send();
		ostr << SubscriptionManager::GetInstance().GetMsg(query["commandID"]);
		response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
		
	} 
	else if(request.getURI().find("/playback/playMedia")!= std::string::npos) {
		std::cout << "playMedia_1" << std::endl;
		AddHeaders(response, request);
				
		std::cout << "playMedia_2" << std::endl;
		std::string protocol = query["protocol"];
		std::string address = query["address"];
		std::string port = query["port"];
		std::string key = query["key"];
		std::cout << "playMedia_3" << std::endl;
		
		std::string fullUrl = protocol + "://" + address + ":" + port + key; // Metainfo
		std::cout << "FullUrl: " << fullUrl << std::endl;
		
		MediaContainer *pCont = Plexservice::GetMediaContainer(fullUrl);
		std::string filePath = pCont->m_vVideos[0].m_pMedia->m_sPartKey;
		Poco::URI fileuri(fullUrl);
		fileuri.setPath(filePath);
		
		// MUSS im Maintread des Plugins/VDR gestartet werden
		ActionManager::GetInstance().AddAction(fileuri.toString());
		
		std::ostream& ostr = response.send();
		ostr << GetOKMsg();
		response.setStatus(Poco::Net::HTTPResponse::HTTP_REASON_OK);
		SubscriptionManager::GetInstance().Notify();
	}
	else if(request.getURI().find("/navigation")!= std::string::npos) {
		if(request.getURI().find("/moveUp")!= std::string::npos) {
			cRemote::Put(eKeys::kUp);
		}
		else if(request.getURI().find("/moveDown")!= std::string::npos) {
			cRemote::Put(eKeys::kDown);
		}
		else if(request.getURI().find("/moveLeft")!= std::string::npos) {
			cRemote::Put(eKeys::kLeft);
		}
		else if(request.getURI().find("/moveRight")!= std::string::npos) {
			cRemote::Put(eKeys::kRight);
		}
		else if(request.getURI().find("/select")!= std::string::npos) {
			cRemote::Put(eKeys::kOk);
		}
		else if(request.getURI().find("/home")!= std::string::npos) {
			cRemote::Put(eKeys::kMenu);
		}
		else if(request.getURI().find("/back")!= std::string::npos) {
			cRemote::Put(eKeys::kBack);
		}
	}
	
}

}
