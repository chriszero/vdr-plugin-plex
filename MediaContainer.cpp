#include "MediaContainer.h"

#ifdef SKINDESIGNER

#include "pictureCache.h"

#endif

namespace plexclient {
    MediaContainer::MediaContainer(std::istream *response) : MediaContainer(response, NULL) { }

    MediaContainer::MediaContainer(std::istream *response, PlexServer *Server) {
        m_pServer = Server;
        m_eViewGroup = MediaType::UNDEF;
        try {
            InputSource src(*response);
            DOMParser parser;
            Poco::XML::AutoPtr<Document> pDoc = parser.parse(&src);

            NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
            Poco::XML::Node *pNode = it.nextNode();
            while (pNode) {
                if (Poco::icompare(pNode->nodeName(), "MediaContainer") == 0) {
                    Poco::XML::NamedNodeMap *pAttribs = pNode->attributes();

                    m_sTitle = GetNodeValue(pAttribs->getNamedItem("title"));
                    m_sTitle1 = GetNodeValue(pAttribs->getNamedItem("title1"));
                    m_sTitle2 = GetNodeValue(pAttribs->getNamedItem("title2"));
                    m_sGrandparentTitle = GetNodeValue(pAttribs->getNamedItem("grandparentTitle"));
                    m_sParentTitle = GetNodeValue(pAttribs->getNamedItem("parentTitle"));
                    m_iParentIndex = GetNodeValueAsInt(pAttribs->getNamedItem("parentIndex"));
                    m_sThumb = GetNodeValue(pAttribs->getNamedItem("thumb"));
                    m_sBanner = GetNodeValue(pAttribs->getNamedItem("banner"));
                    m_eViewGroup = GetNodeValueAsMediaType(pAttribs->getNamedItem("viewGroup"));
                    m_sLibrarySectionTitle = GetNodeValue(pAttribs->getNamedItem("librarySectionTitle"));
                    m_sLibrarySectionUUID = GetNodeValue(pAttribs->getNamedItem("librarySectionUUID"));
                    m_iLibrarySectionID = GetNodeValueAsInt(pAttribs->getNamedItem("librarySectionID"));
                    m_sMediaTagPrefix = GetNodeValue(pAttribs->getNamedItem("mediaTagPrefix"));
                    m_iSize = GetNodeValueAsInt(pAttribs->getNamedItem("size"));
                    m_bAllowSync = GetNodeValueAsBool(pAttribs->getNamedItem("allowSync"));
                    m_sArt = GetNodeValue(pAttribs->getNamedItem("art"));
                    m_sSummary = GetNodeValue(pAttribs->getNamedItem("summary"));
                    m_iParentIndex = GetNodeValueAsInt(pAttribs->getNamedItem("parentIndex"));
                    m_iParentYear = GetNodeValueAsInt(pAttribs->getNamedItem("parentYear"));

                    pAttribs->release();
                } else if (Poco::icompare(pNode->nodeName(), "Directory") == 0) {
                    m_vDirectories.push_back(Directory(pNode, m_pServer, this));
                } else if (Poco::icompare(pNode->nodeName(), "Video") == 0) {
                    m_vVideos.push_back(cVideo(pNode, m_pServer, this));
                } else if (Poco::icompare(pNode->nodeName(), "Device") == 0) {
                    m_vDevices.push_back(Device(pNode, this));
                } else if (Poco::icompare(pNode->nodeName(), "Playlist") == 0) {
                    m_vPlaylists.push_back(Playlist(pNode, this));
                }

                pNode = it.nextNode();
            }

        } catch (Exception &exc) {
            std::cerr << exc.displayText() << std::endl;
        }
    }

    std::string MediaContainer::ArtUri() {
        if (m_sArt.find("http://") != std::string::npos) return m_sArt;
        return m_pServer->GetUri() + m_sArt;
    }

    std::string MediaContainer::ThumbUri() {
        if (m_sThumb.find("http://") != std::string::npos) return m_sThumb;
        return m_pServer->GetUri() + m_sThumb;
    }

#ifdef SKINDESIGNER

    void MediaContainer::PreCache() {
        bool foo;
        for (std::vector<plexclient::cVideo>::iterator it = m_vVideos.begin(); it != m_vVideos.end(); ++it) {
            if (!it->m_sThumb.empty()) cPictureCache::GetInstance().GetPath(it->ThumbUri(), 1280, 720, foo);
            if (!it->m_sArt.empty()) cPictureCache::GetInstance().GetPath(it->ArtUri(), 1920, 1080, foo);
        }
/*	for(std::vector<plexclient::Directory>::iterator it = m_vDirectories.begin(); it != m_vDirectories.end(); ++it) {
		if(!it->m_sThumb.empty()) cPictureCache::GetInstance().GetPath(it->ThumbUri(), 1280, 720, foo);
		if(!it->m_sArt.empty()) cPictureCache::GetInstance().GetPath(it->ArtUri(), 1920, 1080, foo);
	}*/
    }

#endif

}
