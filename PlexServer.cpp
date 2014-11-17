#include <vdr/tools.h>
#include "PlexServer.h"

namespace plexclient
{

PlexServer::PlexServer(std::string data, std::string ip)
{
	m_sIpAddress = ip;
	std::istringstream f(data);
	std::string s;
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
				m_nPort = atoi(val.c_str());
			} else if (name == "Updated-At") {
				m_nUpdated = atol(val.c_str());
			} else if (name == "Version") {
				m_sVersion = val;
			}
		}
	}
}

std::string PlexServer::GetUri() {
	return std::string("http://") + m_sIpAddress + ":" + std::string(itoa(m_nPort));
}

PlexServer::~PlexServer()
{
}

}
