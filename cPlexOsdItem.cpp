#include "cPlexOsdItem.h"

cPlexOsdItem::cPlexOsdItem(const char *title) : cOsdItem(title) {
    m_bVideo = false;
    m_bDir = false;
}

cPlexOsdItem::cPlexOsdItem(const char *title, std::shared_ptr<plexclient::Plexservice> service) : cOsdItem(title) {
    pservice = service;
    m_bVideo = false;
    m_bDir = false;
}

cPlexOsdItem::cPlexOsdItem(const char *title, plexclient::cVideo *obj) : cOsdItem(title) {
    item = obj;
    m_bVideo = true;
    m_bDir = false;
}

cPlexOsdItem::cPlexOsdItem(const char *title, plexclient::Directory *obj) : cOsdItem(title) {
    dir = obj;
    m_bDir = true;
    m_bVideo = false;
}

cPlexOsdItem::cPlexOsdItem(const char *title, plexclient::Stream *obj) : cOsdItem(title) {
    stream = *obj;
    dir = NULL;
    item = NULL;
    pservice = NULL;
    m_bVideo = false;
    m_bDir = false;
}

plexclient::cVideo *cPlexOsdItem::GetAttachedVideo() {
    return item;
}

plexclient::Directory *cPlexOsdItem::GetAttachedDirectory() {
    return dir;
}

std::shared_ptr<plexclient::Plexservice> cPlexOsdItem::GetAttachedService() {
    return pservice;
}
