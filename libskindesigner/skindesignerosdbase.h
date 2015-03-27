#ifndef __SKINDESIGNEROSDBASE_H
#define __SKINDESIGNEROSDBASE_H

using namespace std;

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <vdr/osdbase.h>
#include <vdr/plugin.h>
#include "services.h"

class cOsdView;

/**********************************************************************
* cSkindesignerOsdObject
**********************************************************************/
class cSkindesignerOsdObject : public cOsdObject {
protected:
    string pluginName;
    cPlugin *pSkinDesigner;
    bool InitSkindesignerInterface(string pluginName);
    cOsdView *GetOsdView(int viewID, int subViewID = -1);
public:
    cSkindesignerOsdObject(void);
    virtual ~cSkindesignerOsdObject();
    virtual void Show(void) {};
};

/**********************************************************************
* cSkindesignerOsdItem
**********************************************************************/
class cSkindesignerOsdItem : public cOsdItem {
private:
    cSDDisplayMenu *sdDisplayMenu;
    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > loopTokens;
protected:
public:
    cSkindesignerOsdItem(eOSState State = osUnknown);
    cSkindesignerOsdItem(const char *Text, eOSState State = osUnknown, bool Selectable = true);
    virtual ~cSkindesignerOsdItem();
    virtual void SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable);
    void SetDisplayMenu(cSDDisplayMenu *sdDisplayMenu) { this->sdDisplayMenu = sdDisplayMenu; };
    void AddStringToken(string key, string value);
    void AddIntToken(string key, int value);
    void AddLoopToken(string loopName, map<string, string> &tokens);
};

/**********************************************************************
* cSkindesignerOsdMenu
**********************************************************************/
class cSkindesignerOsdMenu : public cOsdMenu {
private:
    bool init;
    bool displayText;
    string pluginName;
    cSDDisplayMenu *sdDisplayMenu;
    string text;
    map < string, string > stringTokens;
    map < string, int > intTokens;
    map < string, vector< map< string, string > > > loopTokens;
    bool SetSkinDesignerDisplayMenu(void);
protected:
    void ClearTokens(void);
    void SetPluginName(string name) {pluginName = name; };
    void SetPluginMenu(int menu, eMenuType type);
    void SetText(string text) { this->text = text; };
    void AddStringToken(string key, string value);
    void AddIntToken(string key, int value);
    void AddLoopToken(string loopName, map<string, string> &tokens);
    void TextKeyLeft(void);
    void TextKeyRight(void);
    void TextKeyUp(void);
    void TextKeyDown(void);
public:
    cSkindesignerOsdMenu(const char *Title, int c0 = 0, int c1 = 0, int c2 = 0, int c3 = 0, int c4 = 0);
    virtual ~cSkindesignerOsdMenu();
    virtual void Display(void);
};

#endif // __SKINDESIGNEROSDBASE_H

