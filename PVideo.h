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

#ifdef SKINDESIGNER

#include <libskindesignerapi/osdelements.h>
#include "viewGridNavigator.h"
#include "pictureCache.h"

#endif

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

namespace plexclient {
    class MediaContainer;

    class cVideo : private XmlObject
#ifdef SKINDESIGNER
            , public cGridElement
#endif
    {
    private:
        MediaContainer *m_pParent;

        void Parse(Poco::XML::Node *pNode);

        void ParseExtras(Poco::XML::Node *pNode);

    public:
        cVideo(Poco::XML::Node *pNode, PlexServer *Server, MediaContainer *parent);

        cVideo() { };

    public:
        int m_iRatingKey;
        std::string m_sKey;
        std::string m_sStudio;
        MediaType m_tType;
        std::string m_sTitle;
        std::string m_sOriginalTitle;
        std::string m_sGrandparentTitle;
        std::string m_sContentRating;
        std::string m_sSummary;
        std::string m_sTagline;
        long m_lViewoffset;
        Poco::Timestamp m_tLastViewedAt;
        int m_iYear;
        std::string m_sThumb;
        std::string m_sGrandparentThumb;
        std::string m_sArt;
        std::string m_sGrandparentArt;
        long m_iDuration;
        int m_iViewCount;
        double m_dRating;
        Poco::Timestamp m_tAddedAt;
        Poco::Timestamp m_tUpdatedAt;
        Poco::DateTime m_tOriginallyAvailableAt;

        std::vector<std::string> m_vGenre;
        std::vector<std::string> m_vWriter;
        std::vector<std::string> m_vDirector;
        std::vector<std::string> m_vCountry;
        std::vector<std::string> m_vRole;
        std::string m_sCollection;
        Media m_Media;
        PlexServer *m_pServer;
        int m_iMyPlayOffset;
        int m_iIndex;
        int m_iParentIndex;
        std::vector<cVideo> m_vExtras;
        ExtraType m_eExtraType;

        virtual std::string GetTitle();

        bool SetStream(Stream *stream);

        bool UpdateFromServer();

        bool SetWatched();

        bool SetUnwatched();

        std::string ThumbUri();

        std::string ArtUri();

        std::string GetSubtitleUrl();

#ifdef SKINDESIGNER

        // gridElement
        virtual void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> grid, bool clear = true,
                               std::function<void(cGridElement *)> OnCached = NULL);

#endif
    };

}

#endif // VIDEO_H
