#include <vdr/tools.h>
#include "plexgdm.h"

namespace plexclient
{

plexgdm::plexgdm()
{	
	_discoverMessage = "M-SEARCH * HTTP/1.0";
	_clientHeader = "* HTTP/1.0";
	_multicastAddress = "239.0.0.250";
	_clientUpdatePort = 32412;
	
	m_discoverAdress = Poco::Net::SocketAddress(_multicastAddress, 32414);
	m_clientRegisterGroup = Poco::Net::SocketAddress(_multicastAddress, 32413);
	
	m_discoveryComplete = false;
	m_registrationIsRunning = false;
	m_clientRegistered = false;
	m_discoveryInterval = 120;
	m_discoveryIsRunning = false;
}

plexgdm::~plexgdm()
{
}

void plexgdm::clientDetails(std::string c_id, std::string c_name, std::string c_port, std::string c_product, std::string c_version)
{
	_clientData = 	"Content-Type: plex/media-player\n"
					"Resource-Identifier: " + c_id + "\n"
					"Device-Class: HTPC\n"
					"Name: " + c_name + "\n"
					"Port: " + c_port + "\n"
					"Product: " + c_product + "\n"
					"Protocol: plex\n"
					"Protocol-Capabilities: navigation,playback,timeline\n"
					"Protocol-Version: 1\n"
					"Version: " + c_version;
	_clientId = c_id;
}

std::string plexgdm::getClientDetails()
{
	if (_clientData.empty()) throw "client_Data not initialized";
	return _clientData;
}

void plexgdm::Action()
{
	
	char buffer[1024];
	m_registrationIsRunning = true;
	cMutexLock lock(&m_mutex);

try {
	Poco::Net::MulticastSocket update_sock(
	    Poco::Net::SocketAddress( Poco::Net::IPAddress(), m_discoverAdress.port() ),
		true
	);

	update_sock.setLoopback(true);
	update_sock.setReuseAddress(true);
	update_sock.setTimeToLive(255);
	update_sock.setBlocking(false);
	// Send initial Client Registration

	std::string s = Poco::format("HELLO %s\n%s", _clientHeader, _clientData);
	//std::cout << "SendTo:\n" << s << std::endl;
	update_sock.sendTo(s.c_str(), s.length(), m_clientRegisterGroup,0);


	while(m_registrationIsRunning) {
		//std::cout << "cUpd running " << m_registrationIsRunning << std::endl;
		Poco::Net::SocketAddress sender;
		int n = 0;
		try {
			n = update_sock.receiveFrom(buffer, sizeof(buffer), sender);
		} catch (Poco::TimeoutException &e) {
			n = 0;
		}
		if(n > 0) {
			std::string buf(buffer, n);
			if (buf.find("M-SEARCH * HTTP/1.") != std::string::npos) {
				std::cout << "Detected client discovery request from " << sender.host().toString() << " Replying..." << std::endl;
				s = Poco::format("HTTP/1.0 200 OK\n%s", _clientData);
				update_sock.sendTo(s.c_str(), s.length(), sender);
				m_clientRegistered = true;
			}
		}
		m_waitCondition.TimedWait(m_mutex, 600);
	}
	// Client loop stopped

	// unregister from Server
	s = Poco::format("BYE %s\n%s", _clientHeader, _clientData);
	//std::cout << "Unregister: \n" << s << std::endl;
	update_sock.sendTo(s.c_str(), s.length(), m_clientRegisterGroup,0);

	m_clientRegistered = false;
}
	catch(Poco::Exception &exc){
		esyslog("[plex]Exception in %s s%", __func__, exc.displayText().c_str() );
		std::cout << "[plex]Exception: in " << __func__ << " | " << exc.displayText() << std::endl;
	}

}

void plexgdm::discover()
{
	Poco::Net::MulticastSocket socket(
			Poco::Net::SocketAddress( Poco::Net::IPAddress(), m_discoverAdress.port() ),
			true
	);
	// TODO: Discover multiple servers
	char buffer[1024];
	std::map<std::string, std::string> vBuffer;
	
	socket.setLoopback(true);
	socket.setTimeToLive(1);
	socket.setReceiveTimeout(Poco::Timespan(0, 600*1000)); // microseconds
	socket.joinGroup(m_discoverAdress.host());
	socket.sendTo(_discoverMessage.c_str(), _discoverMessage.length(), m_discoverAdress, 0);
	
	try {
		while(true) {
			Poco::Net::SocketAddress sender;
			int n = socket.receiveFrom(buffer, sizeof(buffer), sender);
			std::string buf(buffer, n);
			if(buf.find("200 OK") != std::string::npos) {
				vBuffer[sender.host().toString()] = buf;
			}
			//std::cout << "Discover received from: " << sender.host().toString() << "\nData:\n" << buf << std::endl;
		}
	}
	catch(Poco::TimeoutException &exc){
		//dsyslog("[plex]TimeoutException in %s s%", __func__, exc.displayText().c_str() );
		std::cout << "[plex]TimeoutException: in " << __func__ << " | " << exc.displayText() << std::endl;
	}
	
	socket.close();
	m_discoveryComplete = true;
	
	for(std::map<std::string, std::string>::iterator it = vBuffer.begin() ; it != vBuffer.end(); ++it) {
		std::string host = it->first;
		std::string data = it->second;
		
		m_vServers.push_back(PlexServer(data, host));
	}
}

void plexgdm::stopRegistration()
{
	std::cout << "stop Reg" << std::endl;
	if(m_registrationIsRunning) {
		m_registrationIsRunning = false;
		m_waitCondition.Broadcast();
		Cancel(3);
	}
}


}
