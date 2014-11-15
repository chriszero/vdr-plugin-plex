#include "cPlexOsdItem.h"

cPlexOsdItem::cPlexOsdItem(const char* title) :cOsdItem(title) {
	item = 0;
	dir = 0;
	m_bVideo = false;
	m_bDir = false;
}

cPlexOsdItem::cPlexOsdItem(const char* title, plexclient::Video* obj) :cOsdItem(title) {
	item = obj;
	dir = 0;
	m_bVideo = true;
	m_bDir = false;
}

cPlexOsdItem::cPlexOsdItem(const char* title, plexclient::Directory* obj) :cOsdItem(title) {
	dir = obj;
	item = 0;
	m_bDir = true;
	m_bVideo = false;
}

plexclient::Video* cPlexOsdItem::GetAttachedVideo() {
	return item;
}

plexclient::Directory* cPlexOsdItem::GetAttachedDirectory() {
	return dir;
}

cPlexOsdItem::~cPlexOsdItem()
{
}