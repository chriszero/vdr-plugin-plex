#ifndef PLEXGDM_H
#define PLEXGDM_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/MulticastSocket.h>
#include <Poco/Format.h>

#include <vdr/thread.h>

#include "PlexServer.h"

namespace plexclient
{

class plexgdm : public cThread
{
public:
	plexgdm();
	~plexgdm();
	void clientDetails(std::string c_id, std::string c_name, std::string c_port, std::string c_product, std::string c_version);
	std::string getClientDetails();
	PlexServer* getServerList();
	void discover();;
	void checkClientRegistration();
	
	void Action(void);
	
	//void startAll();
	void startRegistration();

	//void stopAll();
	void stopRegistration();


	PlexServer* GetPServer() {
		return m_pServer;
	}

protected:


	private:
	
	cMutex m_mutex;
	cCondVar m_waitCondition;

	Poco::Net::SocketAddress m_discoverAdress;
	Poco::Net::SocketAddress m_clientRegisterGroup;

	volatile int m_discoveryInterval;

	volatile bool m_discoveryComplete;
	volatile bool m_clientRegistered;
	volatile bool m_discoveryIsRunning;
	volatile bool m_registrationIsRunning;

	std::string _discoverMessage = "M-SEARCH * HTTP/1.0";
	std::string _clientHeader = "* HTTP/1.0";
	std::string _clientData;
	std::string _clientId;
	std::string _multicastAddress = "239.0.0.250";
	int _clientUpdatePort = 32412;
	PlexServer *m_pServer;
};

}
#endif // PLEXGDM_H
