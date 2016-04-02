#ifndef PLEXHELPER_H
#define PLEXHELPER_H

#include <Poco/Net/HTTPRequest.h>

namespace plexclient {

    class PlexHelper {
    public:
        static void AddHttpHeader(Poco::Net::HTTPRequest &request);

    private:
        PlexHelper() { };

    };

}

#endif // PLEXHELPER_H
