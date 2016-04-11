#ifndef PLEXHTTPREQUSTHANDLER_H
#define PLEXHTTPREQUSTHANDLER_H

#include <Poco/Net/HTTPRequestHandler.h> // Base class: Poco::Net::HTTPRequestHandler
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/URI.h>
#include <Poco/StringTokenizer.h>

#include <iostream>
#include <sstream>

#include "plex.h"
#include "Config.h"
#include "SubscriptionManager.h"
#include "Plexservice.h"
#include "MediaContainer.h"
#include "Directory.h"
#include "PVideo.h"
#include "Media.h"

#include "plexgdm.h"


namespace plexclient {

    class PlexHTTPRequestHandler : public Poco::Net::HTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

    protected:
        std::string GetOKMsg();

        void AddHeaders(Poco::Net::HTTPServerResponse &response, Poco::Net::HTTPServerRequest &request);

        std::map<std::string, std::string> ParseQuery(std::string query);

        void UpdateCommandId(Poco::Net::HTTPServerRequest &request);
    };

    class SubscribeRequestHandler : public PlexHTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

        void Subscribe(Poco::Net::HTTPServerRequest &request);

        void Unsubscribe(Poco::Net::HTTPServerRequest &request);
    };

    class ResourceRequestHandler : public PlexHTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    };

    class PlayerRequestHandler : public PlexHTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    };

    class MirrorRequestHandler : public PlexHTTPRequestHandler {
    public:
        virtual void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);
    };
}


#endif // PLEXHTTPREQUSTHANDLER_H
