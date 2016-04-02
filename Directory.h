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

#ifdef SKINDESIGNER

#include <libskindesignerapi/osdelements.h>
#include "pictureCache.h"
#include "viewGridNavigator.h"

#endif

#include "XmlObject.h"
#include "MediaContainer.h"
#include "PlexServer.h"

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient {
    class MediaContainer;

    class Directory : private XmlObject
#ifdef SKINDESIGNER
            , public cGridElement
#endif
    {
    public:
        Directory(Poco::XML::Node *pNode, PlexServer *Server, MediaContainer *parent);

    public:
        bool m_bAllowSync;
        int m_iIndex; // Season, Episode number
        int m_iYear;
        int m_iLeafCount; // Total number of Episodes
        int m_iViewedLeafCount; // Watched Episodes
        int m_iChildCount; // Number of Seasons
        double m_fRating;
        std::string m_sSummary;
        std::string m_sParentSummary;
        std::string m_sTitle;
        std::string m_sTitle1;
        std::string m_sTitle2;
        std::string m_sParentTitle;
        std::string m_sComposite;
        std::string m_sLanguage;
        std::string m_sUuid;
        std::string m_sArt;
        std::string m_sThumb;
        std::string m_sStudio;
        Poco::Timestamp m_tUpdatedAt;
        Poco::Timestamp m_tCreatedAt;
        std::string m_sKey;

        std::vector<std::string> m_vGenre;
        std::vector<std::string> m_vRole;

        MediaType m_eType;
        PlexServer *m_pServer;
        MediaContainer *m_pParent;

        virtual std::string GetTitle();

        std::string ArtUri();

        std::string ThumbUri();

#ifdef SKINDESIGNER

        // gridElement
        virtual void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear = true,
                               std::function<void(cGridElement *)> OnCached = NULL);

#endif
    };

}


#endif // DIRECTORY_H
