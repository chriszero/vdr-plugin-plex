#include "PVideo.h"
#include <Poco/Format.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

#include <vdr/tools.h>
#include "PlexHelper.h"

namespace plexclient
{

Video::Video(Poco::XML::Node* pNode, PlexServer Server, MediaContainer* parent)
{
	m_iMyPlayOffset = 0;
	m_lViewoffset = 0;
	m_Server = Server;
	Parse(pNode);

	if (m_iParentIndex < 0) {
		m_iParentIndex = parent->m_iParentIndex;
	}
}

bool Video::UpdateFromServer()
{
	try {
		Poco::URI fileuri(Poco::format("%s/library/metadata/%d", m_Server.GetUri(), m_iRatingKey));
		Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, fileuri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
		PlexHelper::AddHttpHeader(request);

		Poco::Net::HTTPClientSession session(fileuri.getHost(), fileuri.getPort());

		session.sendRequest(request);
		Poco::Net::HTTPResponse response;
		std::istream &rs = session.receiveResponse(response);

		if(response.getStatus() == 200) {
			// clear vectors
			m_vCountry.clear();
			m_vDirector.clear();
			m_vGenre.clear();
			m_vRole.clear();
			m_vWriter.clear();

			InputSource src(rs);
			DOMParser parser;
			Poco::XML::AutoPtr<Document> pDoc = parser.parse(&src);

			NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
			Poco::XML::Node* pNode = it.nextNode();
			while(pNode) {
				if(Poco::icompare(pNode->nodeName(), "Video") == 0) {
					Parse(pNode);
				}

				pNode = it.nextNode();
			}
			return true;
		}
	} catch (Poco::Exception &exc) {
		esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
	}
	return false;
}

void Video::Parse(Poco::XML::Node* pNode)
{
	NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
	Poco::XML::Node* pChildNode = it.nextNode();

	while(pChildNode) {
		if(Poco::icompare(pChildNode->nodeName(), "Video") == 0) {

			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pNode->attributes();

			m_iRatingKey = GetNodeValueAsInt(pAttribs->getNamedItem("ratingKey"));
			m_iViewCount = GetNodeValueAsInt(pAttribs->getNamedItem("viewCount"));
			m_iIndex = GetNodeValueAsInt(pAttribs->getNamedItem("index"));
			m_iParentIndex = GetNodeValueAsInt(pAttribs->getNamedItem("parentIndex"));
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

std::string Video::GetTitle()
{
	std::string res = m_sTitle;
	switch(m_tType) {
	case MOVIE:
		if(m_iYear > 0) {
			res = Poco::format("%s (%d)", m_sTitle, m_iYear);
		}
		else {
			res = m_sTitle;
		}
		break;
	case EPISODE:
		res = Poco::format("%02dx%02d - %s", m_iParentIndex, m_iIndex, m_sTitle);
		break;
	default:
		break;
	}
	return res;
}

bool Video::SetStream(Stream* stream)
{
	try {
		Poco::Net::HTTPClientSession session(m_Server.GetIpAdress(), m_Server.GetPort());

		std::string uri = Poco::format("/library/parts/%d?%s", m_Media.m_iPartId, stream->GetSetStreamQuery());
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_PUT, uri);
		session.sendRequest(req);

		Poco::Net::HTTPResponse resp;
		session.receiveResponse(resp);

		if(resp.getStatus() == 200) {
			dsyslog("[plex]: Set Stream: %s", uri.c_str());
			return true;
		}
		return false;
	} catch (Poco::Exception &exc) {
		esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
		return false;
	}
}

bool Video::SetUnwatched()
{
	try {
		Poco::Net::HTTPClientSession session(m_Server.GetIpAdress(), m_Server.GetPort());

		std::string uri = Poco::format("/:/unscrobble?key=%d&identifier=com.plexapp.plugins.library", m_iRatingKey);
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, uri);
		session.sendRequest(req);

		Poco::Net::HTTPResponse resp;
		session.receiveResponse(resp);

		if(resp.getStatus() == 200) {
			dsyslog("[plex]: Set Unwatched: %s", uri.c_str());
			return true;
		}
		return false;
	} catch (Poco::Exception &exc) {
		esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
		return false;
	}
}

bool Video::SetWatched()
{
		try {
		Poco::Net::HTTPClientSession session(m_Server.GetIpAdress(), m_Server.GetPort());

		std::string uri = Poco::format("/:/scrobble?key=%d&identifier=com.plexapp.plugins.library", m_iRatingKey);
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, uri);
		session.sendRequest(req);

		Poco::Net::HTTPResponse resp;
		session.receiveResponse(resp);

		if(resp.getStatus() == 200) {
			dsyslog("[plex]: Set Watched: %s", uri.c_str());
			return true;
		}
		return false;
	} catch (Poco::Exception &exc) {
		esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
		return false;
	}
}


} // Namespace
