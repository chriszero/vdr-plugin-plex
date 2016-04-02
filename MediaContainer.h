#ifndef ALLSECTIONS_H
#define ALLSECTIONS_H

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/Exception.h>
#include <Poco/String.h>

#include <vector>
#include <iostream>

#include "XmlObject.h"
#include "Directory.h"
#include "PVideo.h"
#include "PlexServer.h"
#include "Config.h"
#include "device.h"
#include "playlist.h"

using Poco::XML::DOMParser;
using Poco::XML::InputSource;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;

namespace plexclient {
    class cVideo;

    class Directory;

    class Device;

    class Playlist;

    class MediaContainer : XmlObject {
    public:
        MediaContainer(std::istream *response, PlexServer *Server);

        MediaContainer(std::istream *response);

    protected:


    public:
        std::vector<Directory> m_vDirectories;
        std::vector<cVideo> m_vVideos;
        std::vector<Device> m_vDevices;
        std::vector<Playlist> m_vPlaylists;

        bool m_bAllowSync;
        std::string m_sArt;
        std::string m_sThumb;
        std::string m_sBanner;
        std::string m_sTitle;
        std::string m_sTitle1;
        std::string m_sTitle2;
        std::string m_sGrandparentTitle;
        MediaType m_eViewGroup;
        int m_iLibrarySectionID;
        std::string m_sLibrarySectionTitle;
        std::string m_sLibrarySectionUUID;
        std::string m_sMediaTagPrefix;
        int m_iSize;
        std::string m_sSummary;
        int m_iParentIndex;
        std::string m_sParentTitle;
        int m_iParentYear;

        PlexServer *m_pServer;

        std::string ThumbUri();

        std::string ArtUri();

#ifdef SKINDESIGNER

        void PreCache();

#endif
    };

}

#endif // ALLSECTIONS_H
