#include "Directory.h"
#include <vdr/i18n.h>
#include <Poco/Format.h>
#include "pictureCache.h"

namespace plexclient
{

Directory::Directory(Poco::XML::Node* pNode, PlexServer* Server, MediaContainer* parent)
{
	m_pParent = parent;
	m_pServer = Server;

	NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
	Poco::XML::Node* pChildNode = it.nextNode();

	while(pChildNode) {
		if(Poco::icompare(pChildNode->nodeName(), "Directory") == 0) {
			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();

			m_bAllowSync = GetNodeValueAsBool(pAttribs->getNamedItem("allowSync"));
			m_iIndex = GetNodeValueAsInt(pAttribs->getNamedItem("index"));
			m_iLeafCount = GetNodeValueAsInt(pAttribs->getNamedItem("leafCount"));
			m_iViewedLeafCount = GetNodeValueAsInt(pAttribs->getNamedItem("viewedLeafCount"));
			m_iChildCount = GetNodeValueAsInt(pAttribs->getNamedItem("childCount"));
			m_fRating = GetNodeValueAsDouble(pAttribs->getNamedItem("rating"));
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
			m_sSummary = GetNodeValue(pAttribs->getNamedItem("summary"));
			m_sStudio = GetNodeValue(pAttribs->getNamedItem("studio"));

			pAttribs->release();
		} else if(Poco::icompare(pChildNode->nodeName(), "Genre") == 0) {
			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
			m_vGenre.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
			pAttribs->release();
			
		} else if(Poco::icompare(pChildNode->nodeName(), "Role") == 0) {
			Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
			m_vRole.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
			pAttribs->release();
		}
		pChildNode = it.nextNode();
	}
	if(m_sTitle2.empty()) m_sTitle2 = parent->m_sTitle2;
}

std::string Directory::GetTitle()
{
	switch(m_eType) {
	case SEASON:
		return Poco::format(tr("%s - Season %d"), m_sTitle2, m_iIndex);
	default:
		return m_sTitle;
	}
}

void Directory::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
{
	if(clear) grid->ClearTokens();
	grid->AddIntToken("viewmode", Config::GetInstance().DefaultViewMode);
	grid->AddStringToken("title", m_sTitle);
	grid->AddIntToken("viewgroup", m_pParent->m_eViewGroup);

	// Thumb, Cover, Episodepicture
	bool cached = false;
	if(!ThumbUri().empty()) {
		std::string thumb = cPictureCache::GetInstance().GetPath(ThumbUri(), Config::GetInstance().ThumbWidth(), Config::GetInstance().ThumbHeight(), cached, OnCached, this);
		if (cached)	grid->AddStringToken("thumb", thumb);
	}
	grid->AddIntToken("hasthumb", cached);

	// Fanart
	cached = false;
	if(!ArtUri().empty()) {
		std::string art = cPictureCache::GetInstance().GetPath(ArtUri(), Config::GetInstance().ArtWidth(), Config::GetInstance().ArtHeight(), cached);
		if (cached)	grid->AddStringToken("art", art);
	}
	grid->AddIntToken("hasart", cached);

	if(m_eType == MediaType::UNDEF || m_eType == MediaType::MOVIE || m_eType == MediaType::PHOTO) {
		grid->AddIntToken("isdirectory", true);
	}

	if(m_eType == MediaType::SHOW) {
		grid->AddIntToken("isshow", true);
		grid->AddStringToken("summary", m_sSummary);
		grid->AddIntToken("leafCount", m_iLeafCount);
		grid->AddIntToken("viewedLeafCount", m_iViewedLeafCount);
		grid->AddIntToken("childCount", m_iChildCount);
		grid->AddIntToken("rating", m_fRating*10);
		grid->AddStringToken("ratingstring", Poco::format("%.1f", m_fRating));
		grid->AddStringToken("studio", m_sStudio);
		
		map<string, string> roles;
		for(auto it = m_vRole.begin(); it != m_vRole.end(); it++) {
			roles["actor"] = *it;
		}
		grid->AddLoopToken("roles", roles);
		
		map<string, string> gernes;
		for(auto it = m_vGenre.begin(); it != m_vGenre.end(); it++) {
			roles["genre"] = *it;
		}
		grid->AddLoopToken("genres", gernes);
		
		grid->AddIntToken("year", m_iYear);
	}

	if(m_eType == MediaType::SEASON) {
		grid->AddIntToken("isseason", true);
		if(m_pParent) grid->AddStringToken("summary", m_pParent->m_sSummary);
		grid->AddIntToken("season", m_iIndex);
		grid->AddIntToken("leafCount", m_iLeafCount);
		grid->AddIntToken("viewedLeafCount", m_iViewedLeafCount);
		grid->AddStringToken("seriestitle", m_pParent->m_sParentTitle);
		grid->AddIntToken("year", m_pParent->m_iParentYear);
	}

	// Banner, Seriesbanner
	if(m_pParent && !m_pParent->m_sBanner.empty()) {
		cached = false;
		std::string banner = cPictureCache::GetInstance().GetPath(m_pServer->GetUri() + m_pParent->m_sBanner, Config::GetInstance().BannerWidth(), Config::GetInstance().BannerHeight(), cached, OnCached, this);
		if(cached) {
			grid->AddIntToken("hasbanner", cached);
			grid->AddStringToken("banner", banner);
		}
	}
}

std::string Directory::ArtUri()
{
	if(!m_sArt.empty()) {
		if(m_sArt.find("http://") != std::string::npos) return m_sArt;
		if(m_sArt[0] == '/') return m_pServer->GetUri() + m_sArt;
		return m_pServer->GetUri() + '/' + m_sArt;
	}
	if(m_pParent) return m_pParent->ArtUri();
	return "";
}

std::string Directory::ThumbUri()
{
	if(!m_sThumb.empty()) {
		if(m_sThumb.find("http://") != std::string::npos) return m_sThumb;
		if(m_sThumb[0] == '/') return m_pServer->GetUri() + m_sThumb;
		return m_pServer->GetUri() + '/' + m_sThumb;
	}
	if(m_pParent) return m_pParent->ThumbUri();
	return "";
}

}
