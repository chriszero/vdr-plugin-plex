#include "Stream.h"

namespace plexclient
{

Stream::Stream(Poco::XML::Node* pNode)
{	
	if(Poco::icompare(pNode->nodeName(), "Stream") == 0) {

		Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

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

} // namespace
