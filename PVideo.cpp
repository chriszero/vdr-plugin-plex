#include "PVideo.h"
#include <Poco/Format.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <functional>

#include <vdr/tools.h>
#include "PlexHelper.h"
#include "tokendefinitions.h"

namespace plexclient {

    cVideo::cVideo(Poco::XML::Node *pNode, PlexServer *Server, MediaContainer *parent) {
        m_iMyPlayOffset = 0;
        m_lViewoffset = 0;
        m_dRating = 0;
        m_pServer = Server;
        Parse(pNode);

        m_pParent = parent;
        if (m_iParentIndex < 0 && m_pParent) {
            m_iParentIndex = parent->m_iParentIndex;
        }
    }

    bool cVideo::UpdateFromServer() {
        try {
            Poco::URI fileuri(
                    Poco::format("%s/library/metadata/%d?includeExtras=1", m_pServer->GetUri(), m_iRatingKey));
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, fileuri.getPathAndQuery(),
                                           Poco::Net::HTTPMessage::HTTP_1_1);
            PlexHelper::AddHttpHeader(request);

            Poco::Net::HTTPClientSession session(fileuri.getHost(), fileuri.getPort());

            session.sendRequest(request);
            Poco::Net::HTTPResponse response;
            std::istream &rs = session.receiveResponse(response);

            if (response.getStatus() == 200) {
                // clear vectors
                m_vCountry.clear();
                m_vDirector.clear();
                m_vGenre.clear();
                m_vRole.clear();
                m_vWriter.clear();
                m_vExtras.clear();

                InputSource src(rs);
                DOMParser parser;
                Poco::XML::AutoPtr<Document> pDoc = parser.parse(&src);

                NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
                Poco::XML::Node *pNode = it.nextNode();
                while (pNode) {
                    if (Poco::icompare(pNode->nodeName(), "Video") == 0) {
                        Parse(pNode);
                        break;
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

    void cVideo::Parse(Poco::XML::Node *pNode) {
        NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
        Poco::XML::Node *pChildNode = it.nextNode();

        while (pChildNode) {
            if (Poco::icompare(pChildNode->nodeName(), "Video") == 0) {

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
                m_sGrandparentTitle = GetNodeValue(pAttribs->getNamedItem("grandparentTitle"));
                m_sContentRating = GetNodeValue(pAttribs->getNamedItem("contentRating"));
                m_sSummary = GetNodeValue(pAttribs->getNamedItem("summary"));
                m_sTagline = GetNodeValue(pAttribs->getNamedItem("tagline"));
                m_lViewoffset = GetNodeValueAsLong(pAttribs->getNamedItem("viewOffset"));
                m_tLastViewedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("lastViewedAt"));
                m_iYear = GetNodeValueAsInt(pAttribs->getNamedItem("year"));
                m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
                m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));
                m_sGrandparentThumb = GetNodeValue(pAttribs->getNamedItem("grandparentThumb"));
                m_sGrandparentArt = GetNodeValue(pAttribs->getNamedItem("grandparentArt"));
                m_iDuration = GetNodeValueAsLong(pAttribs->getNamedItem("duration"));
                m_dRating = GetNodeValueAsDouble(pAttribs->getNamedItem("rating"));
                m_tAddedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("addedAt"));
                m_tUpdatedAt = GetNodeValueAsTimeStamp(pAttribs->getNamedItem("updatedAt"));
                m_tOriginallyAvailableAt = GetNodeValueAsDateTime(pAttribs->getNamedItem("originallyAvailableAt"));
                m_eExtraType = GetNodeValueAsExtraType(pAttribs->getNamedItem("extraType"));

                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Media") == 0) {
                m_Media = Media(pChildNode);
            } else if (Poco::icompare(pChildNode->nodeName(), "Genre") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vGenre.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Writer") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vWriter.push_back(GetNodeValue(pChildNode));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Director") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vDirector.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Country") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vCountry.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Role") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_vRole.push_back(GetNodeValue(pAttribs->getNamedItem("tag")));
                pAttribs->release();

            } else if (Poco::icompare(pChildNode->nodeName(), "Collection") == 0) {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> pAttribs = pChildNode->attributes();
                m_sCollection = GetNodeValue(pAttribs->getNamedItem("tag"));
                pAttribs->release();
            } else if (Poco::icompare(pChildNode->nodeName(), "Extras") == 0) {
                ParseExtras(pChildNode);
            }
            pChildNode = it.nextNode();
        }
    }

    void cVideo::ParseExtras(Poco::XML::Node *pNode) {
        NodeIterator it(pNode, Poco::XML::NodeFilter::SHOW_ALL);
        Poco::XML::Node *pChildNode = it.nextNode();

        while (pChildNode) {
            if (Poco::icompare(pChildNode->nodeName(), "Video") == 0) {
                m_vExtras.push_back(cVideo(pChildNode, m_pServer, NULL));
            }
            pChildNode = it.nextNode();
        }
    }

    std::string cVideo::GetTitle() {
        std::string res = m_sTitle;

        std::string seriesTitle = m_sGrandparentTitle;

        switch (m_tType) {
            case MediaType::MOVIE:
                if (m_iYear > 0) {
                    res = Poco::format("%s (%d)", m_sTitle, m_iYear);
                } else {
                    res = m_sTitle;
                }
                break;
            case MediaType::EPISODE:
                if (seriesTitle.empty() && m_pParent) seriesTitle = m_pParent->m_sGrandparentTitle;
                res = Poco::format("%s - %02dx%02d - %s", seriesTitle, m_iParentIndex, m_iIndex, m_sTitle);
                break;
            default:
                break;
        }
        return res;
    }

    bool cVideo::SetStream(Stream *stream) {
        try {
            Poco::Net::HTTPClientSession session(m_pServer->GetHost(), m_pServer->GetPort());

            std::string uri = Poco::format("/library/parts/%d?%s&X-Plex-Token=%s", m_Media.m_iPartId,
                                           stream->GetSetStreamQuery(), m_pServer->GetAuthToken());
            Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_PUT, uri);
            session.sendRequest(req);

            Poco::Net::HTTPResponse resp;
            session.receiveResponse(resp);

            if (resp.getStatus() == 200) {
                dsyslog("[plex]: Set Stream: %s", uri.c_str());
                return true;
            }
            return false;
        } catch (Poco::Exception &exc) {
            esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
            return false;
        }
    }

    bool cVideo::SetUnwatched() {
        try {
            std::string uri = Poco::format("/:/unscrobble?key=%d&identifier=com.plexapp.plugins.library", m_iRatingKey);

            bool ok;
            auto cSession = m_pServer->MakeRequest(ok, uri);
            Poco::Net::HTTPResponse resp;
            cSession->receiveResponse(resp);

            if (resp.getStatus() == 200) {
                dsyslog("[plex]: Set Unwatched: %s", uri.c_str());
                return true;
            }
            return false;
        } catch (Poco::Exception &exc) {
            esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
            return false;
        }
    }

    bool cVideo::SetWatched() {
        try {
            std::string uri = Poco::format("/:/scrobble?key=%d&identifier=com.plexapp.plugins.library", m_iRatingKey);

            bool ok;
            auto cSession = m_pServer->MakeRequest(ok, uri);

            if (ok) {
                dsyslog("[plex]: Set Watched: %s", uri.c_str());
                return true;
            }
            return false;
        } catch (Poco::Exception &exc) {
            esyslog("[plex]: %s: %s", __FUNCTION__, exc.displayText().c_str());
            return false;
        }
    }

#ifdef SKINDESIGNER

    void cVideo::AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear,
                           std::function<void(cGridElement *)> OnCached) {
        if (clear) grid->ClearTokens();
        grid->AddIntToken((int) (eTokenGridInt::viewmode), Config::GetInstance().DefaultViewMode);
        grid->AddStringToken((int) (eTokenGridStr::title), m_sTitle.c_str());
        grid->AddStringToken((int) (eTokenGridStr::orginaltitle), m_sOriginalTitle.c_str());
        grid->AddStringToken((int) (eTokenGridStr::summary), m_sSummary.c_str());
        grid->AddStringToken((int) (eTokenGridStr::tagline), m_sTagline.c_str());
        grid->AddStringToken((int) (eTokenGridStr::contentrating), m_sContentRating.c_str());
        grid->AddIntToken((int) (eTokenGridInt::rating), m_dRating * 10);
        grid->AddStringToken((int) (eTokenGridStr::ratingstring), Poco::format("%.1f", m_dRating).c_str());
        grid->AddStringToken((int) (eTokenGridStr::studio), m_sStudio.c_str());
        grid->AddIntToken((int) (eTokenGridInt::viewCount), m_iViewCount);
        grid->AddIntToken((int) (eTokenGridInt::viewoffset), m_lViewoffset / 1000 / 60);
        if (m_iDuration > 0) // avoid division by zero
            grid->AddIntToken((int) (eTokenGridInt::viewoffsetpercent), 100.0 / m_iDuration * m_lViewoffset);
        else
            grid->AddIntToken((int) (eTokenGridInt::viewoffsetpercent), 0);
        grid->AddIntToken((int) (eTokenGridInt::duration), m_iDuration / 1000 / 60);
        grid->AddIntToken((int) (eTokenGridInt::year), m_iYear);
        if (m_pParent) grid->AddIntToken((int) (eTokenGridInt::viewgroup), (int) m_pParent->m_eViewGroup);

        // Thumb, Cover, Episodepicture
        bool cached = false;
        std::string thumb = cPictureCache::GetInstance().GetPath(ThumbUri(), Config::GetInstance().ThumbWidth(),
                                                                 Config::GetInstance().ThumbHeight(), cached, OnCached,
                                                                 this);
        grid->AddIntToken((int) (eTokenGridInt::hasthumb), cached);
        if (cached) grid->AddStringToken((int) (eTokenGridStr::thumb), thumb.c_str());

        // Fanart
        cached = false;
        std::string art = cPictureCache::GetInstance().GetPath(ArtUri(), Config::GetInstance().ArtWidth(),
                                                               Config::GetInstance().ArtHeight(), cached);
        grid->AddIntToken((int) (eTokenGridInt::hasart), cached);
        if (cached) grid->AddStringToken((int) (eTokenGridStr::art), art.c_str());

        if (m_tType == MediaType::MOVIE) {
            grid->AddIntToken((int) (eTokenGridInt::ismovie), true);
        } else if (m_tType == MediaType::CLIP) {
            grid->AddIntToken((int) (eTokenGridInt::isclip), true);
            grid->AddIntToken((int) eTokenGridInt::extratype, (int) m_eExtraType);
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

        grid->AddIntToken((int) (eTokenGridInt::originallyAvailableYear), m_tOriginallyAvailableAt.year());
        grid->AddIntToken((int) (eTokenGridInt::originallyAvailableMonth), m_tOriginallyAvailableAt.month());
        grid->AddIntToken((int) (eTokenGridInt::originallyAvailableDay), m_tOriginallyAvailableAt.day());

        if (m_tType == MediaType::EPISODE) {
            grid->AddIntToken((int) (eTokenGridInt::isepisode), true);
            std::string seriesTitle = m_sGrandparentTitle;
            if (seriesTitle.empty() && m_pParent) seriesTitle = m_pParent->m_sGrandparentTitle;
            grid->AddStringToken((int) (eTokenGridStr::seriestitle), seriesTitle.c_str());
            grid->AddIntToken((int) (eTokenGridInt::season), m_iParentIndex);
            grid->AddIntToken((int) (eTokenGridInt::episode), m_iIndex);

            // Seriescover, Seasoncover
            cached = false;
            std::string grandparentthumbUri = m_sGrandparentThumb;
            if (grandparentthumbUri.empty() && m_pParent) {
                grandparentthumbUri = m_sThumb;
            }
            if (!grandparentthumbUri.empty()) {
                std::string grandparentThumb = cPictureCache::GetInstance().GetPath(
                        m_pServer->GetUri() + grandparentthumbUri, Config::GetInstance().ThumbWidth(),
                        Config::GetInstance().ThumbHeight(), cached, OnCached, this);
                if (cached) grid->AddStringToken((int) (eTokenGridStr::seriesthumb), grandparentThumb.c_str());
            }
            grid->AddIntToken((int) (eTokenGridInt::hasseriesthumb), cached);

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

        m_Media.AddTokens(grid);
    }

#endif

    std::string cVideo::ArtUri() {
        if (m_sArt.find("http://") != std::string::npos) return m_sArt;
        if (m_sArt[0] == '/') return m_pServer->GetUri() + m_sArt;
        return m_pServer->GetUri() + '/' + m_sArt;
    }

    std::string cVideo::ThumbUri() {
        if (m_sThumb.find("http://") != std::string::npos) return m_sThumb;
        if (m_sThumb[0] == '/') return m_pServer->GetUri() + m_sThumb;
        return m_pServer->GetUri() + '/' + m_sThumb;
    }

    std::string cVideo::GetSubtitleUrl() {
        // /video/:/transcode/universal/subtitles
        // Argument? m_sKey?
        //std::string subtitleUrl = m_pServer.GetUri() + "/video/:/transcode/universal/subtitles?" + Config::GetInstance().GetUUID();
        return "";
        // Format is "Mpeg4 Timed Text"
    }

} // Namespace
