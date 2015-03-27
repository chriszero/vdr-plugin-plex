#ifndef __SKINDESIGNERSERVICES_H
#define __SKINDESIGNERSERVICES_H

using namespace std;

#include <string>
#include <vector>
#include <map>

enum eMenuType {
    mtList,
    mtText
};

class cSDDisplayMenu : public cSkinDisplayMenu {
public:
    virtual void SetTitle(const char *Title);
    virtual void SetPluginMenu(string name, int menu, int type, bool init);
    virtual bool SetItemPlugin(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens, int Index, bool Current, bool Selectable);
    virtual bool SetPluginText(map<string,string> *stringTokens, map<string,int> *intTokens, map<string,vector<map<string,string> > > *loopTokens);
};

class cSkinDisplayPlugin {
public:
    cSkinDisplayPlugin(void);
    virtual ~cSkinDisplayPlugin(void);
    virtual void Deactivate(bool hide);
    virtual void Activate(void);
    virtual void ClearViewElement(int id);
    virtual void DisplayViewElement(int id);
    virtual void SetViewElementIntTokens(map<string,int> *intTokens);
    virtual void SetViewElementStringTokens(map<string,string> *stringTokens);
    virtual void SetViewElementLoopTokens(map<string,vector<map<string,string> > > *loopTokens);
    virtual void InitGrids(int viewGridID);
    virtual void SetGrid(int viewGridID, long gridID, double x, double y, double width, double height, map<string,int> *intTokens, map<string,string> *stringTokens);
    virtual void SetGridCurrent(int viewGridID, long gridID, bool current);
    virtual void DeleteGrid(int viewGridID, long gridID);
    virtual void DisplayGrids(int viewGridID);
    virtual void ClearGrids(int viewGridID);
    virtual void SetTabIntTokens(map<string,int> *intTokens);
    virtual void SetTabStringTokens(map<string,string> *stringTokens);
    virtual void SetTabLoopTokens(map<string,vector<map<string,string> > > *loopTokens);
    virtual void SetTabs(void);
    virtual void TabLeft(void);
    virtual void TabRight(void);
    virtual void TabUp(void);
    virtual void TabDown(void);
    virtual void DisplayTabs(void);
    virtual void Flush(void);
    virtual bool ChannelLogoExists(string channelId);
    virtual string GetEpgImagePath(void);
};

/*********************************************************************
* Data Structures for Service Calls
*********************************************************************/

// Data structure for service "RegisterPlugin"
class RegisterPlugin {
public:
    RegisterPlugin(void) {
        name = "";
    };
    void SetMenu(int key, string templateName) {
        menus.insert(pair<int, string>(key, templateName));
    }
    void SetView(int key, string templateName) {
        views.insert(pair<int, string>(key, templateName));     
    }
    void SetSubView(int view, int subView, string templateName) {
        pair<int, string> sub = make_pair(subView, templateName);
        subViews.insert(pair<int, pair<int, string> >(view, sub));     
    }
    void SetViewElement(int view, int viewElement, string name) {
        map< int, map<int, string> >::iterator hit = viewElements.find(view);
        if (hit == viewElements.end()) {
            map<int, string> vE;
            vE.insert(pair<int, string >(viewElement, name));
            viewElements.insert(pair<int, map < int, string > >(view, vE));
        } else {
            (hit->second).insert(pair<int, string >(viewElement, name));
        }
    }
    void SetViewGrid(int view, int viewGrid, string name) {
        map< int, map<int, string> >::iterator hit = viewGrids.find(view);
        if (hit == viewGrids.end()) {
            map<int, string> vG;
            vG.insert(pair<int, string >(viewGrid, name));
            viewGrids.insert(pair<int, map < int, string > >(view, vG));
        } else {
            (hit->second).insert(pair<int, string >(viewGrid, name));
        }
    }
// in
    string name;                                     //name of plugin
    map< int, string > menus;                        //menus as key -> templatename hashmap 
    map< int, string>  views;                        //standalone views as key -> templatename hashmap 
    multimap< int, pair <int, string> >  subViews;   //subviews of standalone views as view -> (subview, templatename) multimap 
    map< int, map <int, string> > viewElements;      //viewelements as key -> (viewelement, viewelementname) hashmap 
    map< int, map <int, string> > viewGrids;         //viewgrids as key -> (viewgrid, viewgridname) hashmap
//out
};

// Data structure for service "GetDisplayMenu"
class GetDisplayMenu {
public:
    GetDisplayMenu(void) {
        displayMenu = NULL;
    };
// in
//out	
    cSDDisplayMenu *displayMenu;
};

// Data structure for service "GetDisplayPlugin"
class GetDisplayPlugin {
public:
    GetDisplayPlugin(void) {
        pluginName = "";
        viewID = -1;
        subViewID = -1;
        displayPlugin = NULL;
    };
// in
    string pluginName;
    int viewID;
    int subViewID;
//out
    cSkinDisplayPlugin *displayPlugin;
};
#endif //__SKINDESIGNERSERVICES_H
