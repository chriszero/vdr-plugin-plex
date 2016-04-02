#ifndef DEVICE_H
#define DEVICE_H

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/Timestamp.h>
#include <Poco/String.h>

#include "XmlObject.h"
#include "MediaContainer.h"

namespace plexclient {
    class Device;

    class Connection : private XmlObject {
    public:
        Connection(Poco::XML::Node *pNode, Device *parent);

        std::string m_sProtocol;
        std::string m_sAddress;
        int m_iPort;
        std::string m_sUri;
        bool m_bLocal;

        Device *m_pParent;

    };

    class Device : private XmlObject {
    public:
        Device(Poco::XML::Node *pNode, MediaContainer *parent);

        std::string m_sName;
        std::string m_sProduct;
        std::string m_sProvides;
        std::string m_sClientIdentifier;
        bool m_bOwned;
        std::string m_sAccessToken;
        bool m_bHttpsRequired;
        bool m_bPublicAddressMatches;
        bool m_bPresence;
        std::string m_sSourceTitle;

        std::vector<Connection> m_vConnections;

        MediaContainer *m_pParent;
    };

}

#endif // DEVICE_H
