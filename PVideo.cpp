#include "PVideo.h"

namespace plexclient
{

Video::Video(Poco::XML::Node* pNode, PlexServer* Server)
{
	m_iMyPlayOffset = 0;
	m_pServer = Server;
	NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
	Poco::XML::Node* pChildNode = it.nextNode();

	while(pChildNode) {
		if(Poco::icompare(pChildNode->nodeName(), "Video") == 0) {

			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

			m_iRatingKey = GetNodeValueAsInt(pAttribs->getNamedItem("ratingKey"));
			m_sKey = GetNodeValue(pAttribs->getNamedItem("key"));
			m_sStudio = GetNodeValue(pAttribs->getNamedItem("studio"));
			m_tType = GetNodeValueAsMediaType(pAttribs->getNamedItem("type"));
			m_sTitle = GetNodeValue(pAttribs->getNamedItem("title"));
			m_sOriginalTitle = GetNodeValue(pAttribs->getNamedItem("originalTitle"));
			m_sContentRating = GetNodeValue(pAttribs->getNamedItem("contentRating"));
			m_sSummary = GetNodeValue(pAttribs->getNamedItem("summary"));
			m_lViewoffset = GetNodeValueAsLong(pAttribs->getNamedItem("viewOffset"));
			m_tLastViewedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("lastViewedAt"));
			m_iYear = GetNodeValueAsInt(pAttribs->getNamedItem("year"));
			m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
			m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));
			m_iDuration = GetNodeValueAsLong(pAttribs->getNamedItem("duration"));
			m_tAddedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("addedAt"));
			m_tUpdatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("updatedAt"));

			pAttribs->release();

		} else if(Poco::icompare(pChildNode->nodeName(), "Media") == 0) {
			m_Media = Media(pChildNode);
		} else if(Poco::icompare(pChildNode->nodeName(), "Genre") == 0) {
			m_vGenre.push_back(GetNodeValue(pChildNode));
		} else if(Poco::icompare(pChildNode->nodeName(), "Writer") == 0) {
			m_vWriter.push_back(GetNodeValue(pChildNode));
		} else if(Poco::icompare(pChildNode->nodeName(), "Director") == 0) {
			m_vDirector.push_back(GetNodeValue(pChildNode));
		} else if(Poco::icompare(pChildNode->nodeName(), "Country") == 0) {
			m_vCountry.push_back(GetNodeValue(pChildNode));
		} else if(Poco::icompare(pChildNode->nodeName(), "Role") == 0) {
			m_vRole.push_back(GetNodeValue(pChildNode));
		} else if(Poco::icompare(pChildNode->nodeName(), "Collection") == 0) {
			m_sCollection = GetNodeValue(pChildNode);
		}
		pChildNode = it.nextNode();
	}
}

}
