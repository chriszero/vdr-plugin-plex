#ifndef CONTROLSERVER_H
#define CONTROLSERVER_H

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>

#include "PlexHTTPRequestHandler.h"
#include "PlexReqHandlerFactory.h"

#include <vdr/thread.h>

namespace plexclient {

    class ControlServer : public cThread {

    public:
        static ControlServer &GetInstance() {
            static ControlServer instance;
            return instance;
        }

        void Stop();

    protected:
        void Action();

    private:
        ControlServer();

        Poco::Net::ServerSocket *m_pSvs;
        Poco::Net::HTTPServer *m_pSrv;

    };

}

#endif // CONTROLSERVER_H
