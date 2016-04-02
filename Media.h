#ifndef MEDIA_H
#define MEDIA_H

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

#ifdef SKINDESIGNER

#include <libskindesignerapi/osdelements.h>

#endif

#include <memory>

#include "XmlObject.h" // Base class: model::XmlObject
#include "Stream.h"


using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient {

    class Media : XmlObject {
    public:
        Media() { };

        Media(Poco::XML::Node *pNode);

    public:
        std::string m_sVideoResolution;
        int m_iId;
        long m_lDuration;
        int m_iBitrate;
        int m_iWidth;
        int m_iHeight;
        std::string m_sAspectRatio;
        int m_iAudioChannels;
        std::string m_sAudioCodec;
        std::string m_sVideoCodec;
        std::string m_sContainer;
        std::string m_VideoFrameRate;

        std::string m_sPartKey;
        int m_iPartId;
        long m_lPartDuration;
        std::string m_sPartFile;
        long m_lPartSize;
        std::string m_sPartContainer;

        std::vector<Stream> m_vStreams;

#ifdef SKINDESIGNER

        void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid);

#endif
    };

}

#endif // MEDIA_H
