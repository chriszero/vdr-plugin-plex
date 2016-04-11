#include <vdr/tools.h>
#include "plexgdm.h"
#include "Config.h"
#include <ctime>
#include "Plexservice.h"

namespace plexclient {

    plexgdm::plexgdm() {
        _discoverMessage = "M-SEARCH * HTTP/1.0";
        _clientHeader = "* HTTP/1.0";
        _multicastAddress = "239.0.0.250";
        _clientUpdatePort = 32412;
        _helloSent = false;
        _discoverInterval = 30 * 1000;
        _discoverTimer.Set(1);

        m_discoverAdress = Poco::Net::SocketAddress(_multicastAddress, 32414);
        m_clientRegisterGroup = Poco::Net::SocketAddress(_multicastAddress, 32413);

        m_discoveryComplete = false;
        m_registrationIsRunning = false;
        m_clientRegistered = false;
        m_discoveryInterval = 120;
        m_discoveryIsRunning = false;
    }

    plexgdm::~plexgdm() {
    }

    void plexgdm::clientDetails(std::string c_id,
                                std::string c_name,
                                std::string c_port,
                                std::string c_product,
                                std::string c_version) {
        _clientData = "Content-Type: plex/media-player\r\n"
                              "Resource-Identifier: " + c_id + "\r\n"
                              "Device-Class: PC\r\n"
                              "Name: " + c_name + "\r\n"
                              "Port: " + c_port + "\r\n"
                              "Product: " + c_product + "\r\n"
                              "Protocol: plex\r\n"
                              "Protocol-Capabilities: navigation,playback,timeline,mirror\r\n"
                              "Protocol-Version: 1\r\n"
                              "Version: " + c_version + "\r\n";
        _clientId = c_id;
    }

    std::string plexgdm::getClientDetails() {
        if (_clientData.empty())
            throw "client_Data not initialized";
        return _clientData;
    }

    void plexgdm::Action() {
        if (Config::GetInstance().UseConfiguredServer) {
            // Adds a Server to vector
            GetServer(Config::GetInstance().s_serverHost, Config::GetInstance().ServerPort);

            if (Config::GetInstance().UsePlexAccount) {
                GetServer(Config::GetInstance().s_serverHost, Config::GetInstance().ServerPort)->SetAuthToken(
                        Plexservice::GetMyPlexToken());
            }
        }

        // Get remote Resources
        Plexservice::UpdateResources();

        char buffer[1024];
        m_registrationIsRunning = true;
        m_discoveryIsRunning = true;
        cMutexLock lock(&m_mutex);

        Poco::Net::MulticastSocket update_sock(Poco::Net::SocketAddress(Poco::Net::IPAddress(), _clientUpdatePort),
                                               true);

        update_sock.setLoopback(true);
        update_sock.setReuseAddress(true);
        update_sock.setTimeToLive(255);
        update_sock.setBlocking(false);

        while (Running()) {
            try {
                if (!_helloSent) {
                    // Send initial Client Registration
                    std::string s = Poco::format("HELLO %s\n%s", _clientHeader, _clientData);
                    update_sock.sendTo(s.c_str(), s.length(), m_clientRegisterGroup, 0);
                    _helloSent = true;
                }
                if (m_registrationIsRunning) {
                    Poco::Net::SocketAddress sender;
                    int n = 0;
                    try {
                        n = update_sock.receiveFrom(buffer, sizeof(buffer), sender);
                    } catch (Poco::TimeoutException &e) {
                        n = 0;
                    }
                    if (n > 0) {
                        std::string buf(buffer, n);
                        if (buf.find("M-SEARCH * HTTP/1.") != std::string::npos) {
                            //dsyslog("[plex]: Detected client discovery request from %s", sender.host().toString().c_str());
                            int t = std::time(0);
                            std::string s = Poco::format("HTTP/1.0 200 OK\r\n%sUpdated-At: %d", _clientData, t);
                            update_sock.sendTo(s.c_str(), s.length(), sender, 0);
                            m_clientRegistered = true;
                        }
                    }
                }

            } catch (Poco::IOException &) {
                // networkconnection is lost, reset
                _helloSent = false;
            } catch (Poco::Exception &) {

            }

            if (m_discoveryIsRunning && _discoverTimer.TimedOut()) {
                discover();
                _discoverTimer.Set(_discoverInterval);
            }
            m_waitCondition.TimedWait(m_mutex, 500);
        }
        // Client loop stopped
        try {
            // unregister from Server
            std::string s = Poco::format("BYE %s\r\n%s", _clientHeader, _clientData);
            update_sock.sendTo(s.c_str(), s.length(), m_clientRegisterGroup, 0);
        } catch (Poco::Exception) { }

        m_clientRegistered = false;
    }

    void plexgdm::discover() {
        try {


            char buffer[1024];
            std::map<std::string, std::string> vBuffer;

            Poco::Net::MulticastSocket socket(Poco::Net::SocketAddress(Poco::Net::IPAddress(), m_discoverAdress.port()),
                                              true);
            socket.setLoopback(true);
            socket.setTimeToLive(1);
            socket.setReceiveTimeout(Poco::Timespan(0, 600 * 1000)); // microseconds
            socket.joinGroup(
                    m_discoverAdress.host()); // Throws a NotFoundException if there is no multicast network device
            socket.sendTo(_discoverMessage.c_str(), _discoverMessage.length(), m_discoverAdress, 0);

            try {

                while (true) {
                    Poco::Net::SocketAddress sender;
                    int n = socket.receiveFrom(buffer, sizeof(buffer), sender);
                    std::string buf(buffer, n);
                    if (buf.find("200 OK") != std::string::npos) {
                        vBuffer[sender.host().toString()] = buf;
                    }
                }

            } catch (Poco::TimeoutException) { }

            socket.close();

            m_discoveryComplete = true;

            for (std::map<std::string, std::string>::iterator it = vBuffer.begin(); it != vBuffer.end(); ++it) {
                std::string host = it->first;
                std::string data = it->second;

                PlexServer newServ(data, host);
                // Set token for local servers
                if (Config::GetInstance().UsePlexAccount) {
                    newServ.SetAuthToken(Plexservice::GetMyPlexToken());
                }

                if (AddServer(newServ)) {
                    isyslog("[plex] New server found via GDM: %s", host.c_str());
                }
                else if (GetServer(newServ.m_sUuid)) {
                    GetServer(newServ.m_sUuid)->ParseData(data, host);
                    dsyslog("[plex] Server updated via GDM: %s", host.c_str());
                }
            }
        } catch (Poco::IOException &) {
            // No Networkconnection
        } catch (Poco::NotFoundException &) {
            // there is no multicast network device
        } catch (Poco::Exception &) { }
    }

    void plexgdm::stopRegistration() {
        if (m_registrationIsRunning) {
            m_registrationIsRunning = false;
            m_waitCondition.Broadcast();
            Cancel();
        }
    }

    PlexServer *plexgdm::GetServer(std::string ip, int port) {
        for (std::vector<PlexServer>::iterator s_it = m_vServers.begin(); s_it != m_vServers.end(); ++s_it) {
            if (s_it->GetHost() == ip && s_it->GetPort() == port) {
                return &(*s_it);
            }
        }
        m_vServers.push_back(PlexServer(ip, port));
        return &m_vServers[m_vServers.size() - 1];
    }

/*
 * Returns the first owned online server, if there is no owned server it will return the first remote server, or NULL
 */
    PlexServer *plexgdm::GetFirstServer() {
        for (std::vector<PlexServer>::iterator s_it = m_vServers.begin(); s_it != m_vServers.end(); ++s_it) {
            if (s_it->IsOwned() && !s_it->Offline) {
                return &(*s_it);
            }
        }
        if (m_vServers.size() > 0) return &m_vServers[0];
        return NULL;
    }

    PlexServer *plexgdm::GetServer(std::string uuid) {
        for (std::vector<PlexServer>::iterator s_it = m_vServers.begin(); s_it != m_vServers.end(); ++s_it) {
            if (s_it->GetUuid() == uuid) {
                return &(*s_it);
            }
        }
        return NULL;
    }

    bool plexgdm::AddServer(PlexServer server) {
        for (std::vector<PlexServer>::iterator s_it = m_vServers.begin(); s_it != m_vServers.end(); ++s_it) {
            if (s_it->GetUuid() == server.GetUuid()) {
                return false;
            }
        }
        m_vServers.push_back(server);
        isyslog("[plex] New Server Added: %s", server.GetUri().c_str());
        return true;
    }

} // namespace
