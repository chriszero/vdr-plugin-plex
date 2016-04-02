#ifndef PLEXSERVER_H
#define PLEXSERVER_H

#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include <Poco/String.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/URI.h>

namespace plexclient {

    class PlexServer {
        friend class plexgdm;

    public:
        PlexServer(std::string uri, std::string name, std::string uuid, std::string accessToken, bool owned,
                   bool local);

        ~PlexServer();

        int GetMaster() const {
            return m_nMaster;
        }

        int IsOwned() const {
            return m_nOwned;
        }

        const std::string &GetContentType() const {
            return m_sContentType;
        }

        const std::string &GetDiscovery() const {
            return m_sDiscovery;
        }

        const std::string &GetRole() const {
            return m_sRole;
        }

        const std::string &GetServerName() const {
            return m_sServerName;
        }

        long GetUpdated() const {
            return m_nUpdated;
        }

        const std::string &GetUuid() const {
            return m_sUuid;
        }

        const std::string &GetVersion() const {
            return m_sVersion;
        }

        const std::string &GetAuthToken() const {
            return m_authToken;
        }

        const bool &IsLocal() const {
            return m_bLocal;
        }

        void SetAuthToken(std::string token) {
            m_authToken = token;
        }

        std::shared_ptr<Poco::Net::HTTPClientSession> MakeRequest(bool &ok, std::string path,
                                                                  const std::map<std::string, std::string> &queryParameters = std::map<std::string, std::string>());

        std::string GetHost();

        int GetPort();

        std::string GetUri();

        std::shared_ptr<Poco::Net::HTTPClientSession> GetClientSession();

        bool Offline;

    protected:
        PlexServer(std::string data, std::string ip);

        PlexServer(std::string ip, int port);

        PlexServer() { };

        void ParseData(std::string data, std::string ip);

    private:
        std::string m_sDiscovery;

        int m_nOwned;
        bool m_bLocal;
        int m_nMaster;
        std::string m_sRole;
        std::string m_sContentType;
        std::string m_sUuid;
        std::string m_sServerName;
        std::string m_uri;
        std::string m_authToken;
        long m_nUpdated;
        std::string m_sVersion;
    };

}
#endif // PLEXSERVER_H
