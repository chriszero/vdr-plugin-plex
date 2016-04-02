#ifndef CPLEXOSDITEM_H
#define CPLEXOSDITEM_H

#include <memory>

#include <vdr/osd.h> // Base class: cOsdItem
#include <vdr/interface.h>
#include <vdr/plugin.h>

#include "PVideo.h"
#include "Stream.h"
#include "Directory.h"
#include "Plexservice.h"

class cPlexOsdItem : public cOsdItem {
private:
    plexclient::cVideo *item;
    plexclient::Directory *dir;
    plexclient::Stream stream;
    std::shared_ptr<plexclient::Plexservice> pservice;
    bool m_bVideo;
    bool m_bDir;

public:
    cPlexOsdItem(const char *title);

    cPlexOsdItem(const char *title, std::shared_ptr<plexclient::Plexservice> service);

    cPlexOsdItem(const char *title, plexclient::cVideo *obj);

    cPlexOsdItem(const char *title, plexclient::Directory *obj);

/**
 * @brief 
 * @param title Title
 * @param obj will be copied
 */
    cPlexOsdItem(const char *title, plexclient::Stream *obj);

    plexclient::cVideo *GetAttachedVideo();

    plexclient::Directory *GetAttachedDirectory();

    plexclient::Stream &GetAttachedStream() { return stream; }

    std::shared_ptr<plexclient::Plexservice> GetAttachedService();

    bool IsVideo() const {
        return m_bVideo;
    }

    bool IsDir() const {
        return m_bDir;
    }

    //virtual eOSState ProcessKey(eKeys Key);
    //virtual void Set(void);
    //virtual void SetMenuItem(cSkinDisplayMenu* DisplayMenu, int Index, bool Current, bool Selectable);
};

#endif // CPLEXOSDITEM_H
