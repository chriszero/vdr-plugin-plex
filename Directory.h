#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/Timestamp.h>
#include <Poco/String.h>

#include "XmlObject.h"

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient
{

class Directory: XmlObject
{
public:
	Directory(Poco::XML::Node* pNode);
	~Directory();

public:
	bool m_bAllowSync;
	std::string m_sTitle;
	std::string m_sComposite;
	std::string m_sLanguage;
	std::string m_sUuid;
	std::string m_sArt;
	std::string m_sThumb;
	Poco::Timestamp m_tUpdatedAt;
	Poco::Timestamp m_tCreatedAt;
	std::string m_sKey;
	MediaType m_eType;

};

}


#endif // DIRECTORY_H
