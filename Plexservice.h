#ifndef PLEXSERVICE_H
#define PLEXSERVICE_H

//#include "plexgdm.h"
#include "PlexServer.h"

#include <sstream>
#include <iostream>
#include <string>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <time.h>

#include <Poco/Base64Encoder.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/Context.h>
#include <Poco/Exception.h>
#include <Poco/Format.h>
#include <Poco/URI.h>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/Format.h>

#include <Poco/StreamCopier.h>

#include "Config.h"
#include "user.h"
#include "MediaContainer.h"

namespace plexclient {

    class Plexservice {
    public:
        Plexservice(PlexServer *server);

        Plexservice(PlexServer *server, std::string startUri);

        std::shared_ptr<MediaContainer> GetSection(std::string section, bool putOnStack = true);

        std::shared_ptr<MediaContainer> GetLastSection(bool current = false);

        bool IsRoot();

        PlexServer *GetServer();

        void Authenticate();

        static std::string GetUniversalTranscodeUrl(cVideo *video, int offset = 0, PlexServer *server = 0,
                                                    bool http = false);

        static std::string GetMyPlexToken();

        static std::shared_ptr<MediaContainer> GetMediaContainer(std::string fullUrl);

        //static std::string encode(std::string message);
        static void UpdateResources();

        std::string StartUri;

    private:
        Poco::Mutex m_mutex;
        PlexServer *pServer;

        std::stack<std::string> m_vUriStack;

        std::unique_ptr<Poco::Net::HTTPRequest> CreateRequest(std::string path);

    };

}

#endif // PLEXSERVICE_H
