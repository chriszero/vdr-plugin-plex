#ifndef PLEXREQHANDLERFACTORY_H
#define PLEXREQHANDLERFACTORY_H

#include <Poco/Net/HTTPRequestHandlerFactory.h> // Base class: Poco::Net::HTTPRequestHandlerFactory
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "PlexHTTPRequestHandler.h"

#include "Config.h"

namespace plexclient {

    class PlexReqHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        PlexReqHandlerFactory();

        ~PlexReqHandlerFactory();

    public:
        virtual Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request);
    };

}


#endif // PLEXREQHANDLERFACTORY_H
