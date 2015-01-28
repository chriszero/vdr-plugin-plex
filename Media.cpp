#include "Media.h"

namespace plexclient
{

Media::Media(Poco::XML::Node* pNode)
{
	NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
	Poco::XML::Node* pChildNode = it.nextNode();

	while(pChildNode) {
		if(Poco::icompare(pChildNode->nodeName(), "Media") == 0) {

			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
			m_sVideoResolution = GetNodeValue(pAttribs->getNamedItem("videoResolution"));
			m_iId = GetNodeValueAsInt(pAttribs->getNamedItem("id"));
			m_lDuration = GetNodeValueAsLong(pAttribs->getNamedItem("duration"));
			m_iBitrate = GetNodeValueAsInt(pAttribs->getNamedItem("bitrate"));
			m_iWidth = GetNodeValueAsInt(pAttribs->getNamedItem("width"));
			m_iHeight = GetNodeValueAsInt(pAttribs->getNamedItem("height"));
			m_sAspectRatio = GetNodeValue(pAttribs->getNamedItem("aspectRatio"));
			m_iAudioChannels = GetNodeValueAsInt(pAttribs->getNamedItem("audioChannels"));
			m_sAudioCodec = GetNodeValue(pAttribs->getNamedItem("audioCodec"));
			m_sVideoCodec = GetNodeValue(pAttribs->getNamedItem("videoCodec"));
			m_sContainer = GetNodeValue(pAttribs->getNamedItem("container"));
			m_VideoFrameRate = GetNodeValueAsDouble(pAttribs->getNamedItem("videoFrameRate"));


			pAttribs->release();

		}
		if(Poco::icompare(pChildNode->nodeName(), "Part") == 0) {
			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
			m_sPartKey = GetNodeValue(pAttribs->getNamedItem("key"));
			m_iPartId = GetNodeValueAsInt(pAttribs->getNamedItem("id"));
			m_lPartDuration = GetNodeValueAsLong(pAttribs->getNamedItem("duration"));
			m_sPartFile = GetNodeValue(pAttribs->getNamedItem("file"));
			m_lPartSize = GetNodeValueAsLong(pAttribs->getNamedItem("size"));
			m_sPartContainer = GetNodeValue(pAttribs->getNamedItem("container"));

			pAttribs->release();
		}
		if(Poco::icompare(pChildNode->nodeName(), "Stream") == 0) {
			m_vStreams.push_back(Stream(pChildNode));
		}
		pChildNode = it.nextNode();
	}
}

}
