#ifndef VIDEO_H
#define VIDEO_H

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

#include "XmlObject.h"
#include "Media.h"
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
	
class Video: XmlObject
{
public:
	Video(Poco::XML::Node* pNode, PlexServer* Server);

public:
	int m_iRatingKey;
	std::string m_sKey;
	std::string m_sStudio;
	MediaType m_tType;
	std::string m_sTitle;
	std::string m_sOriginalTitle;
	std::string m_sContentRating;
	std::string m_sSummary;
	long m_lViewoffset;
	Poco::Timestamp m_tLastViewedAt;
	int m_iYear;
	std::string m_sThumb;
	std::string m_sArt;
	long m_iDuration;
	Poco::Timestamp m_tAddedAt;
	Poco::Timestamp m_tUpdatedAt;

	std::vector<std::string> m_vGenre;
	std::vector<std::string> m_vWriter;
	std::vector<std::string> m_vDirector;
	std::vector<std::string> m_vCountry;
	std::vector<std::string> m_vRole;
	std::string m_sCollection;
	Media m_Media;
	PlexServer* m_pServer;
};

}

#endif // VIDEO_H
