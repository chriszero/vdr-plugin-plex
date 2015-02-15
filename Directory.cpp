#include "Directory.h"
#include <Poco/Format.h>

namespace plexclient
{

Directory::Directory(Poco::XML::Node* pNode, MediaContainer* parent)
{
	if(Poco::icompare(pNode->nodeName(), "Directory") == 0) {
		Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

		m_bAllowSync = GetNodeValueAsBool(pAttribs->getNamedItem("allowSync"));
		m_iIndex = GetNodeValueAsInt(pAttribs->getNamedItem("index"));
		m_iYear = GetNodeValueAsInt(pAttribs->getNamedItem("year"));
		m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));
		m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
		m_sKey = GetNodeValue(pAttribs->getNamedItem("key"));
		m_sTitle = GetNodeValue(pAttribs->getNamedItem("title"));
		m_sTitle1 = GetNodeValue(pAttribs->getNamedItem("title1"));
		m_sTitle2 = GetNodeValue(pAttribs->getNamedItem("title2"));
		m_sComposite = GetNodeValue(pAttribs->getNamedItem("composite"));
		m_sLanguage = GetNodeValue(pAttribs->getNamedItem("language"));
		m_sUuid = GetNodeValue(pAttribs->getNamedItem("uuid"));
		m_tUpdatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("updatedAt"));
		m_tCreatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("createdAt"));
		m_eType = GetNodeValueAsMediaType(pAttribs->getNamedItem("type"));

		pAttribs->release();
	}
	if(m_sTitle2.empty()) m_sTitle2 = parent->m_sTitle2;
}

std::string Directory::GetTitle()
{
	switch(m_eType) {
	case SEASON:
		return Poco::format("%s - Staffel %d", m_sTitle2, m_iIndex);
	default:
		return m_sTitle;
	}
}

}
