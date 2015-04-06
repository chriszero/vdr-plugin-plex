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
		return Poco::format(tr("%s - Season %d"), m_sTitle2, m_iIndex);
	default:
		return m_sTitle;
	}
}

void Directory::AddTokens(std::shared_ptr<cOsdElement> grid, bool clear, std::function<void(cGridElement*)> OnCached)
{
	if(clear) grid->ClearTokens();
	grid->AddStringToken("title", m_sTitle);

	// Thumb, Cover, Episodepicture
	bool cached = false;
	std::string thumb = cPictureCache::GetInstance().GetPath(ThumbUri(), Config::GetInstance().ThumbWidth(), Config::GetInstance().ThumbHeight(), cached, OnCached, this);
	grid->AddIntToken("hasthumb", cached);
	if (cached)	grid->AddStringToken("thumb", thumb);

	// Fanart
	cached = false;
	std::string art = cPictureCache::GetInstance().GetPath(ArtUri(), Config::GetInstance().ArtWidth(), Config::GetInstance().ArtHeight(), cached);
	grid->AddIntToken("hasart", cached);
	if (cached)	grid->AddStringToken("art", art);

	if(m_eType == MediaType::SHOW) {
		grid->AddIntToken("isshow", true);
	}

	if(m_eType == MediaType::SEASON) {
		grid->AddIntToken("isseason", true);
	}
	
	// Banner, Seriesbanner
	if(m_pParent && !m_pParent->m_sBanner.empty()) {
		cached = false;
		std::string banner = cPictureCache::GetInstance().GetPath(m_pServer->GetUri() + m_pParent->m_sBanner, Config::GetInstance().BannerWidth(), Config::GetInstance().BannerHeight(), cached, OnCached, this);
		if(cached) {
			grid->AddIntToken("hasbanner", true);
			grid->AddStringToken("banner", banner);
		}
	}
}

std::string Directory::ArtUri()
{
	if(m_sArt.find("http://") != std::string::npos) return m_sArt;
	return m_pServer->GetUri() + m_sArt;
}

std::string Directory::ThumbUri()
{
	if(m_sThumb.find("http://") != std::string::npos) return m_sThumb;
	return m_pServer->GetUri() + m_sThumb;
}

}
