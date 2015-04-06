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

#include <memory>
#include "libskindesigner/osdelements.h"

#include "XmlObject.h"
#include "MediaContainer.h"
#include "viewGridNavigator.h"
#include "PlexServer.h"

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient
{
class MediaContainer;
	
class Directory: private XmlObject, public cGridElement
{	
public:
	Directory(Poco::XML::Node* pNode, PlexServer* Server, MediaContainer* parent);

public:
	bool m_bAllowSync;
	int m_iIndex;
	int m_iYear;
	std::string m_sTitle;
	std::string m_sTitle1;
	std::string m_sTitle2;
	std::string m_sComposite;
	std::string m_sLanguage;
	std::string m_sUuid;
	std::string m_sArt;
	std::string m_sThumb;
	Poco::Timestamp m_tUpdatedAt;
	Poco::Timestamp m_tCreatedAt;
	std::string m_sKey;
	MediaType m_eType;
	PlexServer* m_pServer;
	MediaContainer* m_pParent;
		
	virtual std::string GetTitle();
	std::string ArtUri();
	std::string ThumbUri();
	// gridElement
	virtual void AddTokens(std::shared_ptr<cOsdElement> grid, bool clear = true, std::function<void(cGridElement*)> OnCached = NULL);
};

}


#endif // DIRECTORY_H
