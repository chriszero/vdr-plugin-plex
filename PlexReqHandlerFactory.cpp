#include "PlexReqHandlerFactory.h"

namespace plexclient {

    PlexReqHandlerFactory::PlexReqHandlerFactory() {
    }

    PlexReqHandlerFactory::~PlexReqHandlerFactory() {
    }

    Poco::Net::HTTPRequestHandler *PlexReqHandlerFactory::createRequestHandler(
            const Poco::Net::HTTPServerRequest &request) {
	/*
	if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
		std::cout << "GET Request: " << request.getURI() << " from: " << request.clientAddress().toString() << std::endl;
	} else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
		std::cout << "OPTIONS Request: " << request.getURI() << " from: " << request.clientAddress().toString() << std::endl;
	} else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_HEAD) {
		std::cout << "HEAD Request: " << request.getURI() << " from: " << request.clientAddress().toString() << std::endl;
	}
	else {
		std::cout << "??? Request: " << request.getURI() << " from: " << request.clientAddress().toString() << std::endl;
	}
	*/
        if (request.getURI().find("/player/timeline") != std::string::npos) return new SubscribeRequestHandler();
		else if (request.getURI().find("/player/mirror") != std::string::npos) return new MirrorRequestHandler();
        else if (request.getURI().find("/resources") != std::string::npos) return new ResourceRequestHandler();
        else if (request.getURI().find("/player") != std::string::npos) return new PlayerRequestHandler();

        return new PlexHTTPRequestHandler();
    }

}
