#include <vdr/tools.h>
#include "PlexServer.h"

namespace plexclient
{
	
PlexServer::PlexServer(std::string ip, int port)
{
	m_httpSession = NULL;
	
	Poco::URI uri;
	uri.setHost(ip);
	uri.setPort(port);
	uri.setScheme("http");
	m_uri = uri.toString();
}

PlexServer::PlexServer(std::string data, std::string ip)
{
	m_httpSession = NULL;
	ParseData(data, ip);
}

PlexServer::PlexServer(std::string uri, std::string name, std::string uuid, std::string accessToken, bool owned, bool local)
{
	m_httpSession = NULL;
	m_sServerName = name;
	m_sUuid = uuid;
	m_nOwned = owned;
	m_bLocal = local;
	m_authToken = accessToken;
	
	m_uri = uri;
}

PlexServer::~PlexServer()
{
	delete m_httpSession;
	m_httpSession = NULL;
}

void PlexServer::ParseData(std::string data, std::string ip)
{
	int port = 0;
	std::istringstream f(data);
	std::string s;
	Offline = false;
	while(std::getline(f, s)) {
		int pos = s.find(':');
		if(pos > 0) {
			std::string name = Poco::trim(s.substr(0, pos));
			std::string val = Poco::trim(s.substr(pos+1));
			if(name == "Content-Type") {
				m_sContentType = val;
			} else if (name == "Resource-Identifier") {
				m_sUuid = val;
			} else if (name == "Name") {
				m_sServerName = val;
			} else if (name == "Port") {
				port = atoi(val.c_str());
			} else if (name == "Updated-At") {
				m_nUpdated = atol(val.c_str());
			} else if (name == "Version") {
				m_sVersion = val;
			}
		}
	}
	delete m_httpSession;
	m_httpSession = NULL;
	
	Poco::URI uri;
	
	uri.setHost(ip);
	uri.setPort(port);
	uri.setScheme("http");
	
	m_uri = uri.toString();
}

std::string PlexServer::GetHost()
{
	Poco::URI uri(m_uri);
	return uri.getHost();
}

int PlexServer::GetPort()
{
	Poco::URI uri(m_uri);
	return uri.getPort();
}

Poco::Net::HTTPClientSession* PlexServer::GetClientSession()
{
	Poco::URI uri(m_uri);
	if(m_httpSession == NULL) {
		if(uri.getScheme().find("https") != std::string::npos) {
			m_httpSession = new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort());
		}
		else {
			m_httpSession = new Poco::Net::HTTPClientSession(uri.getHost(), uri.getPort());
		}
	}
	return m_httpSession;
}

std::string PlexServer::GetUri()
{
	return  m_uri;
}

}
