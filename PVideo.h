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
#include <memory>
#include "libskindesigner/osdelements.h"
#include "viewGridNavigator.h"

#include "XmlObject.h"
#include "MediaContainer.h"
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
class MediaContainer;
	
class Video: private XmlObject, public cGridElement
{
private:
	void Parse(Poco::XML::Node* pNode);
	
public:
	Video(Poco::XML::Node* pNode, PlexServer Server, MediaContainer* parent);
	Video() {};

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
	int m_iViewCount;
	Poco::Timestamp m_tAddedAt;
	Poco::Timestamp m_tUpdatedAt;

	std::vector<std::string> m_vGenre;
	std::vector<std::string> m_vWriter;
	std::vector<std::string> m_vDirector;
	std::vector<std::string> m_vCountry;
	std::vector<std::string> m_vRole;
	std::string m_sCollection;
	Media m_Media;
	PlexServer m_Server;
	int m_iMyPlayOffset;
	int m_iIndex;
	int m_iParentIndex;
	
	virtual std::string GetTitle();
	bool SetStream(Stream* stream);
	bool UpdateFromServer();
	bool SetWatched();
	bool SetUnwatched();
	
	// gridElement
	virtual void AddTokens(std::shared_ptr<cViewGrid> grid);
};

}

#endif // VIDEO_H
