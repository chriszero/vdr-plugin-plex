#ifndef XMLOBJECT_H
#define XMLOBJECT_H

#include <cstdlib>

#include <Poco/DOM/Document.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/String.h>
#include <Poco/Exception.h>

namespace plexclient {

    enum class MediaType {
        UNDEF = 0, MOVIE, SHOW, SEASON, EPISODE, MUSIC, PHOTO, CLIP, PLAYLIST
    };
    enum class PlaylistType {
        Undef, Video, Audio, Photo
    };
    enum class StreamType {
        sUNDEF = 0, sVIDEO = 1, sAUDIO = 2, sSUBTITLE = 3
    };
    enum class ExtraType {
        Unkown = 0, Trailer = 1, BehindTheScenes = 5
    };

    class XmlObject {
    protected:
        static std::string GetNodeValue(Poco::XML::Node *pNode);

        static int GetNodeValueAsInt(Poco::XML::Node *pNode);

        static long GetNodeValueAsLong(Poco::XML::Node *pNode);

        static double GetNodeValueAsDouble(Poco::XML::Node *pNode);

        static bool GetNodeValueAsBool(Poco::XML::Node *pNode);

        static Poco::Timestamp GetNodeValueAsTimeStamp(Poco::XML::Node *pNode);

        static Poco::DateTime GetNodeValueAsDateTime(Poco::XML::Node *pNode);

        static MediaType GetNodeValueAsMediaType(Poco::XML::Node *pNode);

        static StreamType GetNodeValueAsStreamType(Poco::XML::Node *pNode);

        static PlaylistType GetNodeValueAsPlaylistType(Poco::XML::Node *pNode);

        static ExtraType GetNodeValueAsExtraType(Poco::XML::Node *pNode);

    private:
    };

}

#endif // XMLOBJECT_H
