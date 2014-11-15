#include "MediaContainer.h"

namespace plexclient
{
MediaContainer::MediaContainer(std::istream* response)
{
	try {
		InputSource src(*response);
		DOMParser parser;
		Poco::XML::AutoPtr<Document> pDoc = parser.parse(&src);

		NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
		Poco::XML::Node* pNode = it.nextNode();

		while(pNode) {
			if(Poco::icompare(pNode->nodeName(), "MediaContainer") == 0) {
				Poco::XML::NamedNodeMap* pAttribs = pNode->attributes();

				m_sTitle = GetNodeValue(pAttribs->getNamedItem("title1"));
				m_sTitle2 = GetNodeValue(pAttribs->getNamedItem("title2"));
				m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
				m_sViewGroup = GetNodeValue(pAttribs->getNamedItem("viewGroup"));
				m_sLibrarySectionTitle = GetNodeValue(pAttribs->getNamedItem("librarySectionTitle"));
				m_sLibrarySectionUUID = GetNodeValue(pAttribs->getNamedItem("librarySectionUUID"));
				m_iLibrarySectionID = GetNodeValueAsInt(pAttribs->getNamedItem("librarySectionID"));
				m_sMediaTagPrefix = GetNodeValue(pAttribs->getNamedItem("mediaTagPrefix"));
				m_iSize = GetNodeValueAsInt(pAttribs->getNamedItem("size"));
				m_bAllowSync = GetNodeValueAsBool(pAttribs->getNamedItem("allowSync"));
				m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));

				pAttribs->release();
			} else if(Poco::icompare(pNode->nodeName(), "Directory") == 0) {
				m_vDirectories.push_back(Directory(pNode));
			} else if(Poco::icompare(pNode->nodeName(), "Video") == 0) {
				m_vVideos.push_back(Video(pNode));
			}

			pNode = it.nextNode();
		}

	} catch(Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
	}
}

}
