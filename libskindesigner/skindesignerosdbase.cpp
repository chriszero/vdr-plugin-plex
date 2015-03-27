#include "skindesignerosdbase.h"
#include "osdelements.h"

using namespace std;

/**********************************************************************
* cSkindesignerOsdObject
**********************************************************************/

cSkindesignerOsdObject::cSkindesignerOsdObject(void) {
    pSkinDesigner = NULL;
    pluginName = "";
}

cSkindesignerOsdObject::~cSkindesignerOsdObject() {
}

bool cSkindesignerOsdObject::InitSkindesignerInterface(string pluginName) {
    this->pluginName = pluginName;
    pSkinDesigner = cPluginManager::GetPlugin("skindesigner");
    if (!pSkinDesigner) {
        return false;   
    }
    return true;
}

cOsdView *cSkindesignerOsdObject::GetOsdView(int viewID, int subViewID) {
    cSkinDisplayPlugin *displayPlugin = NULL;
    cOsdView *view = NULL;
    GetDisplayPlugin call;
    call.pluginName = pluginName;
    call.viewID = viewID;
    call.subViewID = subViewID;
    bool ok = pSkinDesigner->Service("GetDisplayPlugin", &call);
    if (ok) {
        displayPlugin = call.displayPlugin;
        view = new cOsdView(displayPlugin);
    }
    return view;
}

/**********************************************************************
* cSkindesignerOsdItem
**********************************************************************/
cSkindesignerOsdItem::cSkindesignerOsdItem(eOSState State) : cOsdItem(State) {
    sdDisplayMenu = NULL;
}

cSkindesignerOsdItem::cSkindesignerOsdItem(const char *Text, eOSState State, bool Selectable) : cOsdItem(Text, State, Selectable) {
    sdDisplayMenu = NULL;
}  

cSkindesignerOsdItem::~cSkindesignerOsdItem() {

}

void cSkindesignerOsdItem::SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable) {
    if (sdDisplayMenu) {
        if (!sdDisplayMenu->SetItemPlugin(&stringTokens, &intTokens, &loopTokens, Index, Current, Selectable)) {
            DisplayMenu->SetItem(Text(), Index, Current, Selectable);
        }
    } else {
        DisplayMenu->SetItem(Text(), Index, Current, Selectable);
    }
}

void cSkindesignerOsdItem::AddStringToken(string key, string value) {
    stringTokens.insert(pair<string,string>(key, value));
}

void cSkindesignerOsdItem::AddIntToken(string key, int value) {
    intTokens.insert(pair<string,int>(key, value));
}

void cSkindesignerOsdItem::AddLoopToken(string loopName, map<string, string> &tokens) {
    map<string, vector<map<string, string> > >::iterator hitLoop = loopTokens.find(loopName);
    if (hitLoop == loopTokens.end()) {
        vector<map<string, string> > tokenVector;
        tokenVector.push_back(tokens);
        loopTokens.insert(pair<string, vector<map<string, string> > >(loopName, tokenVector));
    } else {
        vector<map<string, string> > *tokenVector = &hitLoop->second;
        tokenVector->push_back(tokens);
    }
}


/**********************************************************************
* cSkindesignerOsdMenu
**********************************************************************/
cSkindesignerOsdMenu::cSkindesignerOsdMenu(const char *Title, int c0, int c1, int c2, int c3, int c4) : cOsdMenu(Title, c0, c1, c2, c3, c4) {
    init = true;
    displayText = false;
    sdDisplayMenu = NULL;
    pluginName = "";
    SetMenuCategory(mcPlugin);
    SetSkinDesignerDisplayMenu();
}

cSkindesignerOsdMenu::~cSkindesignerOsdMenu() {

}

void cSkindesignerOsdMenu::SetPluginMenu(int menu, eMenuType type) {
    if (type == mtList)
        displayText = false;
    else if (type == mtText)
        displayText = true;

    if (sdDisplayMenu) {
        sdDisplayMenu->SetPluginMenu(pluginName, menu, type, init);
    }
    init = false;
}

bool cSkindesignerOsdMenu::SetSkinDesignerDisplayMenu(void) {
    static cPlugin *pSkinDesigner = cPluginManager::GetPlugin("skindesigner");
    if (!pSkinDesigner) {
        return false;   
    }
    GetDisplayMenu call;
    bool ok = pSkinDesigner->Service("GetDisplayMenu", &call);
    if (ok && call.displayMenu) {
        sdDisplayMenu = call.displayMenu;
        return true;
    }
    return false;
}

void cSkindesignerOsdMenu::ClearTokens(void) {
    text = "";
    stringTokens.clear();
    intTokens.clear();
    loopTokens.clear();
}

void cSkindesignerOsdMenu::AddStringToken(string key, string value) {
    stringTokens.insert(pair<string,string>(key, value));
}

void cSkindesignerOsdMenu::AddIntToken(string key, int value) {
    intTokens.insert(pair<string,int>(key, value));
}

void cSkindesignerOsdMenu::AddLoopToken(string loopName, map<string, string> &tokens) {
    map<string, vector<map<string, string> > >::iterator hitLoop = loopTokens.find(loopName);
    if (hitLoop == loopTokens.end()) {
        vector<map<string, string> > tokenVector;
        tokenVector.push_back(tokens);
        loopTokens.insert(pair<string, vector<map<string, string> > >(loopName, tokenVector));
    } else {
        vector<map<string, string> > *tokenVector = &hitLoop->second;
        tokenVector->push_back(tokens);
    }
}

void cSkindesignerOsdMenu::TextKeyLeft(void) {
    if (!displayText)
        return;
    DisplayMenu()->Scroll(true, true);
}

void cSkindesignerOsdMenu::TextKeyRight(void) {
    if (!displayText)
        return;
    DisplayMenu()->Scroll(false, true);
}

void cSkindesignerOsdMenu::TextKeyUp(void) {
    if (!displayText)
        return;
    DisplayMenu()->Scroll(true, false);
}

void cSkindesignerOsdMenu::TextKeyDown(void) {
    if (!displayText)
        return;
    DisplayMenu()->Scroll(false, false);
}

void cSkindesignerOsdMenu::Display(void) {
    if (displayText) {
        if (sdDisplayMenu) {
            sdDisplayMenu->SetTitle(Title());
            if (sdDisplayMenu->SetPluginText(&stringTokens, &intTokens, &loopTokens)) {
                sdDisplayMenu->Flush();
            } else {
                DisplayMenu()->Clear();
                DisplayMenu()->SetTitle(Title());
                DisplayMenu()->SetText(text.c_str(), false);
                DisplayMenu()->Flush();
            }
        } else {
            DisplayMenu()->Clear();
            DisplayMenu()->SetTitle(Title());
            DisplayMenu()->SetText(text.c_str(), false);
            DisplayMenu()->Flush();
        }
        return;
    }
    if (sdDisplayMenu) {
        sdDisplayMenu->SetTitle(Title());
        for (cOsdItem *item = First(); item; item = Next(item)) {
            cSkindesignerOsdItem *sdItem = dynamic_cast<cSkindesignerOsdItem*>(item);
            if (sdItem) {
                sdItem->SetDisplayMenu(sdDisplayMenu);
            }
        }
    }
    cOsdMenu::Display();
}
