#ifndef STREAM_H
#define STREAM_H


#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/Timestamp.h>
#include <Poco/String.h>

#include <vector>
#include <iostream>

#include "XmlObject.h" // Base class: model::XmlObject

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient {

    class Stream : XmlObject {
    public:
        Stream(Poco::XML::Node *pNode);

        Stream() { };

    public:
        bool m_bSelected;
        int m_iID;
        int m_iStreamType;
        int m_iIndex;
        int m_iChannels;
        std::string m_sCodec;
        std::string m_sCodecId;
        std::string m_sLanguage;
        std::string m_sLanguageCode;
        StreamType m_eStreamType;

        std::string GetSetStreamQuery();
    };

}

#endif // STREAM_H
