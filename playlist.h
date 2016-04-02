#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "XmlObject.h" // Base class: plexclient::XmlObject
#include "MediaContainer.h"

namespace plexclient {

    class Playlist : private plexclient::XmlObject {
    public:
        Playlist(Poco::XML::Node *pNode, MediaContainer *parent);

    };

}

#endif // PLAYLIST_H
