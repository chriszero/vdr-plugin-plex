#include "displayReplaySD.h"

cDisplayReplaySD::cDisplayReplaySD(plexclient::cVideo *video) : cSkindesignerOsdObject(GetPluginStruct()) {

}

cDisplayReplaySD::~cDisplayReplaySD() {
}

skindesignerapi::cPluginStructure *cDisplayReplaySD::m_pPlugStructReplay = NULL;

skindesignerapi::cPluginStructure *cDisplayReplaySD::GetPluginStruct() {
    if (m_pPlugStructReplay == NULL) {
        m_pPlugStructReplay = new skindesignerapi::cPluginStructure();
        m_pPlugStructReplay->name = "plexreplay";
        m_pPlugStructReplay->libskindesignerAPIVersion = LIBSKINDESIGNERAPIVERSION;
        m_pPlugStructReplay->RegisterRootView("root.xml");
    }
    return m_pPlugStructReplay;
}

void cDisplayReplaySD::Show(void) {
}

eOSState cDisplayReplaySD::ProcessKey(eKeys Key) {
    return eOSState::osContinue;
}

void cDisplayReplaySD::Flush() {
}

void cDisplayReplaySD::SetCurrent(const char *Current) {
}

void cDisplayReplaySD::SetMode(bool Play, bool Forward, int Speed) {
}
