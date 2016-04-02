#include "ControlServer.h"

namespace plexclient {

    ControlServer::ControlServer() {
        m_pSvs = new Poco::Net::ServerSocket(3200);;
        m_pSrv = new Poco::Net::HTTPServer(new PlexReqHandlerFactory, *m_pSvs, new Poco::Net::HTTPServerParams);
    }

    void ControlServer::Stop() {
        Cancel(1);
        // Stop the HTTPServer
        m_pSrv->stop();
    }

    void ControlServer::Action() {
        isyslog("[plex] Starting Controlserver...");
        // start the HTTPServer
        m_pSrv->start();
        while (Running()) {
            SubscriptionManager::GetInstance().Notify();
            cCondWait::SleepMs(1000);
        }
    }


}
