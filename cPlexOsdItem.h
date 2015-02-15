#ifndef CPLEXOSDITEM_H
#define CPLEXOSDITEM_H

#include <vdr/osd.h> // Base class: cOsdItem
#include <vdr/interface.h>
#include <vdr/plugin.h>

#include "PVideo.h"
#include "Directory.h"
#include "PlexServer.h"

class cPlexOsdItem : public cOsdItem
{
private:
	plexclient::Video *item;
	plexclient::Directory *dir;
	plexclient::PlexServer *pserver;
	bool m_bVideo;	
	bool m_bDir;

public:
	cPlexOsdItem(const char* title);
	cPlexOsdItem(const char* title, plexclient::PlexServer* server);
	cPlexOsdItem(const char* title, plexclient::Video* obj);
	cPlexOsdItem(const char* title, plexclient::Directory* obj);
	plexclient::Video* GetAttachedVideo();
	plexclient::Directory* GetAttachedDirectory();
	plexclient::PlexServer* GetAttachedServer();
	
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
