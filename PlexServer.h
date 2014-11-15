#ifndef PLEXSERVER_H
#define PLEXSERVER_H

#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <Poco/String.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/URI.h>

namespace plexclient
{

class PlexServer
{
public:
	PlexServer(std::string data, std::string ip);
	~PlexServer();

	int GetMaster() const {
		return m_nMaster;
	}

	int GetOwned() const {
		return m_nOwned;
	}
	int GetPort() const {
		return m_nPort;
	}
	const std::string& GetContentType() const {
		return m_sContentType;
	}
	const std::string& GetDiscovery() const {
		return m_sDiscovery;
	}
	const std::string& GetRole() const {
		return m_sRole;
	}
	const std::string& GetServerName() const {
		return m_sServerName;
	}
	long GetUpdated() const {
		return m_nUpdated;
	}
	const std::string& GetUuid() const {
		return m_sUuid;
	}
	const std::string& GetVersion() const {
		return m_sVersion;
	}
	const std::string& GetIpAdress() const {
		return m_sIpAddress;
	}
	
	std::string GetUri();

	void DiscoverSettings();

private:
	std::string m_sDiscovery;

	int m_nOwned = 0;
	int m_nMaster = 0;
	std::string m_sRole;
	std::string m_sContentType;
	std::string m_sUuid;
	std::string m_sServerName;
	std::string m_sIpAddress;
	int m_nPort;
	long m_nUpdated;
	std::string m_sVersion;

};

}
#endif // PLEXSERVER_H
