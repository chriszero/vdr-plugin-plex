#include "Stream.h"
#include <Poco/Format.h>

namespace plexclient {

    Stream::Stream(Poco::XML::Node *pNode) {
        if (Poco::icompare(pNode->nodeName(), "Stream") == 0) {

            Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

            m_bSelected = GetNodeValueAsBool(pAttribs->getNamedItem("selected"));
            m_iID = GetNodeValueAsInt(pAttribs->getNamedItem("id"));
            m_iStreamType = GetNodeValueAsInt(pAttribs->getNamedItem("streamType"));
            m_iIndex = GetNodeValueAsInt(pAttribs->getNamedItem("index"));
            m_iChannels = GetNodeValueAsInt(pAttribs->getNamedItem("channels"));
            m_sCodec = GetNodeValue(pAttribs->getNamedItem("codec"));
            m_sCodecId = GetNodeValue(pAttribs->getNamedItem("codecID"));
            m_sLanguage = GetNodeValue(pAttribs->getNamedItem("language"));
            m_sLanguageCode = GetNodeValue(pAttribs->getNamedItem("languageCode"));
            m_eStreamType = GetNodeValueAsStreamType(pAttribs->getNamedItem("streamType"));

            pAttribs->release();
        }
    }

    std::string Stream::GetSetStreamQuery() {
        if (m_eStreamType == StreamType::sAUDIO) return Poco::format("audioStreamID=%d", m_iID);
        else if (m_eStreamType == StreamType::sSUBTITLE && m_iID >= 0)
            return Poco::format("subtitleStreamID=%d", m_iID);
        else if (m_eStreamType == StreamType::sSUBTITLE && m_iID < 0) return "subtitleStreamID=";
        else return "";
    }

} // namespace
