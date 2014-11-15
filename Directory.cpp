#include "Directory.h"

namespace plexclient
{

Directory::Directory(Poco::XML::Node* pNode)
{
	if(Poco::icompare(pNode->nodeName(), "Directory") == 0) {

		Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

		m_bAllowSync = GetNodeValueAsBool(pAttribs->getNamedItem("allowSync"));
		m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));
		m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
		m_sKey = GetNodeValue(pAttribs->getNamedItem("key"));
		m_sTitle = GetNodeValue(pAttribs->getNamedItem("title"));
		m_sComposite = GetNodeValue(pAttribs->getNamedItem("composite"));
		m_sLanguage = GetNodeValue(pAttribs->getNamedItem("language"));
		m_sUuid = GetNodeValue(pAttribs->getNamedItem("uuid"));
		m_tUpdatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("updatedAt"));
		m_tCreatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("createdAt"));
		m_eType = GetNodeValueAsMediaType(pAttribs->getNamedItem("type"));

		pAttribs->release();
	}
}

Directory::~Directory()
{
}


}

