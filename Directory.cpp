#include "Directory.h"
#include <vdr/i18n.h>
#include <Poco/Format.h>
#include "tokendefinitions.h"

namespace plexclient {

    Directory::Directory(Poco::XML::Node *pNode, PlexServer *Server, MediaContainer *parent) {
        m_pParent = parent;
        m_pServer = Server;

        NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
        Poco::XML::Node *pChildNode = it.nextNode();

        while (pChildNode) {
            if (Poco::icompare(pChildNode->nodeName(), "Directory") == 0) {
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
                m_sParentTitle = GetNodeValue(pAttribs->getNamedItem("parentTitle"));
                m_sComposite = GetNodeValue(pAttribs->getNamedItem("composite"));
                m_sLanguage = GetNodeValue(pAttribs->getNamedItem("language"));
                m_sUuid = GetNodeValue(pAttribs->getNamedItem("uuid"));
                m_tUpdatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("updatedAt"));
                m_tCreatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("createdAt"));
                m_eType = GetNodeValueAsMediaType(pAttribs->getNamedItem("type"));
                m_sSummary = GetNodeValue(pAttribs->getNamedItem("summary"));
                m_sParentSummary = GetNodeValue(pAttribs->getNamedItem("parentSummary"));
                m_sStudio = GetNodeValue(pAttribs->getNamedItem("studio"));

                pAttribs->release();
            } else if (Poco::icompare(pChildNode->nodeName(), "Genre") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vGenre.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Role") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vRole.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();
            }
            pChildNode = it.nextNode();
        }
        if (m_sTitle2.empty()) m_sTitle2 = parent->m_sTitle2;
    }

    std::string Directory::GetTitle() {
        std::string seriesTitle = m_sParentTitle;
        if (seriesTitle.empty() && m_pParent)
            seriesTitle = m_pParent->m_sParentTitle;

        switch (m_eType) {
            case MediaType::SEASON:
                return Poco::format(tr("%s - Season %d"), seriesTitle, m_iIndex);
            default:
                return m_sTitle;
        }
    }

#ifdef SKINDESIGNER

    void Directory::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear,
                              std::function<void(cGridElement *)> OnCached) {
        if (clear) grid->ClearTokens();
        grid->AddIntToken((int) (eTokenGridInt::viewmode), Config::GetInstance().DefaultViewMode);
        grid->AddStringToken((int) (eTokenGridStr::title), m_sTitle.c_str());
        grid->AddIntToken((int) (eTokenGridInt::viewgroup), (int) m_pParent->m_eViewGroup);

        // Thumb, Cover, Episodepicture
        bool cached = false;
        if (!ThumbUri().empty()) {
            std::string thumb = cPictureCache::GetInstance().GetPath(ThumbUri(), Config::GetInstance().ThumbWidth(),
                                                                     Config::GetInstance().ThumbHeight(), cached,
                                                                     OnCached, this);
            if (cached) grid->AddStringToken((int) (eTokenGridStr::thumb), thumb.c_str());
        }
        grid->AddIntToken((int) (eTokenGridInt::hasthumb), cached);

        // Fanart
        cached = false;
        if (!ArtUri().empty()) {
            std::string art = cPictureCache::GetInstance().GetPath(ArtUri(), Config::GetInstance().ArtWidth(),
                                                                   Config::GetInstance().ArtHeight(), cached);
            if (cached) grid->AddStringToken((int) (eTokenGridStr::art), art.c_str());
        }
        grid->AddIntToken((int) (eTokenGridInt::hasart), cached);

        if (m_eType == MediaType::UNDEF || m_eType == MediaType::MOVIE || m_eType == MediaType::PHOTO) {
            grid->AddIntToken((int) (eTokenGridInt::isdirectory), true);
        }

        vector<int> loopInfo;
        loopInfo.push_back(m_vRole.size());
        loopInfo.push_back(m_vGenre.size());
        grid->SetLoop(loopInfo);

        int actloopIndex = grid->GetLoopIndex("roles");
        int i = 0;
        for (auto it = m_vRole.begin(); it != m_vRole.end(); it++) {
            grid->AddLoopToken(actloopIndex, i, (int) (eTokenGridActorLst::roles), it->c_str());
            i++;
        }

        int genloopIndex = grid->GetLoopIndex("genres");
        i = 0;
        for (auto it = m_vGenre.begin(); it != m_vGenre.end(); it++) {
            grid->AddLoopToken(genloopIndex, i, (int) (eTokenGridGenresLst::genres), it->c_str());
            i++;
        }

        if (m_eType == MediaType::SHOW) {
            grid->AddIntToken((int) (eTokenGridInt::isshow), true);
            grid->AddStringToken((int) (eTokenGridStr::summary), m_sSummary.c_str());
            grid->AddIntToken((int) (eTokenGridInt::leafCount), m_iLeafCount);
            grid->AddIntToken((int) (eTokenGridInt::viewedLeafCount), m_iViewedLeafCount);
            grid->AddIntToken((int) (eTokenGridInt::childCount), m_iChildCount);
            grid->AddIntToken((int) (eTokenGridInt::rating), m_fRating * 10);
            grid->AddStringToken((int) (eTokenGridStr::ratingstring), Poco::format("%.1f", m_fRating).c_str());
            grid->AddStringToken((int) (eTokenGridStr::studio), m_sStudio.c_str());

            grid->AddIntToken((int) (eTokenGridInt::year), m_iYear);
        }

        if (m_eType == MediaType::SEASON) {
            grid->AddIntToken((int) (eTokenGridInt::isseason), true);

            std::string summary = m_sParentSummary;
            if (m_sParentSummary.empty() && m_pParent)
                summary = m_pParent->m_sSummary;

            grid->AddStringToken((int) (eTokenGridStr::summary), summary.c_str());
            grid->AddIntToken((int) (eTokenGridInt::season), m_iIndex);
            grid->AddIntToken((int) (eTokenGridInt::leafCount), m_iLeafCount);
            grid->AddIntToken((int) (eTokenGridInt::viewedLeafCount), m_iViewedLeafCount);

            std::string seriesTitle = m_sParentTitle;
            if (seriesTitle.empty() && m_pParent)
                seriesTitle = m_pParent->m_sParentTitle;

            grid->AddStringToken((int) (eTokenGridStr::seriestitle), seriesTitle.c_str());
            grid->AddIntToken((int) (eTokenGridInt::year), m_pParent->m_iParentYear);
        }

        // Banner, Seriesbanner
        if (m_pParent && !m_pParent->m_sBanner.empty()) {
            cached = false;
            std::string banner = cPictureCache::GetInstance().GetPath(m_pServer->GetUri() + m_pParent->m_sBanner,
                                                                      Config::GetInstance().BannerWidth(),
                                                                      Config::GetInstance().BannerHeight(), cached,
                                                                      OnCached, this);
            if (cached) {
                grid->AddIntToken((int) (eTokenGridInt::hasbanner), cached);
                grid->AddStringToken((int) (eTokenGridStr::banner), banner.c_str());
            }
        }
    }

#endif

    std::string Directory::ArtUri() {
        if (!m_sArt.empty()) {
            if (m_sArt.find("http://") != std::string::npos) return m_sArt;
            if (m_sArt[0] == '/') return m_pServer->GetUri() + m_sArt;
            return m_pServer->GetUri() + '/' + m_sArt;
        }
        if (m_pParent) return m_pParent->ArtUri();
        return "";
    }

    std::string Directory::ThumbUri() {
        if (!m_sThumb.empty()) {
            if (m_sThumb.find("http://") != std::string::npos) return m_sThumb;
            if (m_sThumb[0] == '/') return m_pServer->GetUri() + m_sThumb;
            return m_pServer->GetUri() + '/' + m_sThumb;
        }
        if (m_pParent) return m_pParent->ThumbUri();
        return "";
    }

}
