#include "device.h"

namespace plexclient {

    Connection::Connection(Poco::XML::Node *pNode, Device *parent) {
        m_pParent = parent;

        NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
        Poco::XML::Node *pChildNode = it.nextNode();

        while (pChildNode) {
            if (Poco::icompare(pChildNode->nodeName(), "Connection") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();

                m_sProtocol = GetNodeValue(pAttribs->getNamedItem("protocol"));
                m_sAddress = GetNodeValue(pAttribs->getNamedItem("address"));
                m_iPort = GetNodeValueAsInt(pAttribs->getNamedItem("port"));
                m_sUri = GetNodeValue(pAttribs->getNamedItem("uri"));
                m_bLocal = GetNodeValueAsBool(pAttribs->getNamedItem("local"));

                pAttribs->release();
            }
            pChildNode = it.nextNode();
        }
    }

    Device::Device(Poco::XML::Node *pNode, MediaContainer *parent) {
        m_pParent = parent;

        NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
        Poco::XML::Node *pChildNode = it.nextNode();

        while (pChildNode) {
            if (Poco::icompare(pChildNode->nodeName(), "Device") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();

                m_sName = GetNodeValue(pAttribs->getNamedItem("name"));
                m_sProduct = GetNodeValue(pAttribs->getNamedItem("product"));
                m_sProvides = GetNodeValue(pAttribs->getNamedItem("provides"));
                m_sClientIdentifier = GetNodeValue(pAttribs->getNamedItem("clientIdentifier"));
                m_bOwned = GetNodeValueAsBool(pAttribs->getNamedItem("owned"));
                m_sAccessToken = GetNodeValue(pAttribs->getNamedItem("accessToken"));
                m_bHttpsRequired = GetNodeValueAsBool(pAttribs->getNamedItem("httpsRequired"));
                m_bPublicAddressMatches = GetNodeValueAsBool(pAttribs->getNamedItem("publicAddressMatches"));
                m_bPresence = GetNodeValueAsBool(pAttribs->getNamedItem("presence"));
                m_sSourceTitle = GetNodeValue(pAttribs->getNamedItem("sourceTitle"));

                pAttribs->release();
            } else if (Poco::icompare(pChildNode->nodeName(), "Connection") == 0) {
                m_vConnections.push_back(Connection(pChildNode, this));
            }

            pChildNode = it.nextNode();
        }
    }


}
