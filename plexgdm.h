#ifndef PLEXGDM_H
#define PLEXGDM_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/MulticastSocket.h>
#include <Poco/Format.h>

#include <vdr/thread.h>
#include <vdr/tools.h>

#include "PlexServer.h"

namespace plexclient {

    class plexgdm : public cThread {
    public:
        static plexgdm &GetInstance() {
            static plexgdm instance;
            return instance;
        }

        ~plexgdm();

        void clientDetails(std::string c_id, std::string c_name, std::string c_port, std::string c_product,
                           std::string c_version);

        std::string getClientDetails();

        PlexServer *GetServer(std::string ip, int port);

        PlexServer *GetServer(std::string uuid);

        bool AddServer(PlexServer server);

        PlexServer *GetFirstServer();

        void discover();

        void Action(void);

        void stopRegistration();

        std::vector<PlexServer> &GetPlexservers() {
            return m_vServers;
        }

    protected:


    private:
        plexgdm();

        cMutex m_mutex;
        cCondVar m_waitCondition;
        int _discoverInterval;
        cTimeMs _discoverTimer;
        bool _helloSent;

        Poco::Net::SocketAddress m_discoverAdress;
        Poco::Net::SocketAddress m_clientRegisterGroup;

        volatile int m_discoveryInterval;

        volatile bool m_discoveryComplete;
        volatile bool m_clientRegistered;
        volatile bool m_discoveryIsRunning;
        volatile bool m_registrationIsRunning;

        std::string _discoverMessage;
        std::string _clientHeader;
        std::string _clientData;
        std::string _clientId;
        std::string _multicastAddress;
        int _clientUpdatePort;
        std::vector<PlexServer> m_vServers;
    };

}
#endif // PLEXGDM_H
