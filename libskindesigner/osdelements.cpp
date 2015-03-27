#include "osdelements.h"

using namespace std;
	
/**********************************************************************
* cOsdElement
**********************************************************************/
cOsdElement::cOsdElement(cSkinDisplayPlugin *view) {
    this->view = view;
}

cOsdElement::~cOsdElement() {
}

void cOsdElement::ClearTokens(void) {
    stringTokens.clear();
    intTokens.clear();
    loopTokens.clear();
}

void cOsdElement::AddStringToken(string key, string value) {
    stringTokens.insert(pair<string,string>(key, value));
}

void cOsdElement::AddIntToken(string key, int value) {
    intTokens.insert(pair<string,int>(key, value));
}

void cOsdElement::AddLoopToken(string loopName, map<string, string> &tokens) {
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

bool cOsdElement::ChannelLogoExists(string channelId) {
    return view->ChannelLogoExists(channelId);
}

string cOsdElement::GetEpgImagePath(void) {
    return view->GetEpgImagePath();    
}


/**********************************************************************
* cViewElement
**********************************************************************/
cViewElement::cViewElement(cSkinDisplayPlugin *view, int viewElementID) : cOsdElement(view) {
    this->viewElementID = viewElementID;
}

cViewElement::~cViewElement() {
}

void cViewElement::Clear(void) {
    if (!view)
        return;
    view->ClearViewElement(viewElementID);
}

void cViewElement::Display(void) {
    if (!view)
        return;
    view->SetViewElementIntTokens(&intTokens);
    view->SetViewElementStringTokens(&stringTokens);
    view->SetViewElementLoopTokens(&loopTokens);
    view->DisplayViewElement(viewElementID);
}

/**********************************************************************
* cViewGrid
**********************************************************************/
cViewGrid::cViewGrid(cSkinDisplayPlugin *view, int viewGridID) : cOsdElement(view) {
    this->viewGridID = viewGridID;
}

cViewGrid::~cViewGrid() {
}

void cViewGrid::SetGrid(long gridID, double x, double y, double width, double height) {
    if (!view)
        return;
    view->SetGrid(viewGridID, gridID, x, y, width, height, &intTokens, &stringTokens);
}

void cViewGrid::SetCurrent(long gridID, bool current) {
    if (!view)
        return;
    view->SetGridCurrent(viewGridID, gridID, current);
}

void cViewGrid::MoveGrid(long gridID, double x, double y, double width, double height) {
    if (!view)
        return;
    view->SetGrid(viewGridID, gridID, x, y, width, height, NULL, NULL);
}

void cViewGrid::Delete(long gridID) {
    if (!view)
        return;
    view->DeleteGrid(viewGridID, gridID);
}

void cViewGrid::Clear(void) {
    if (!view)
        return;
    view->ClearGrids(viewGridID);    
}

void cViewGrid::Display(void) {
    if (!view)
        return;
    view->DisplayGrids(viewGridID);
}

/**********************************************************************
* cViewTab
**********************************************************************/
cViewTab::cViewTab(cSkinDisplayPlugin *view) : cOsdElement(view) {
}

cViewTab::~cViewTab() {
}

void cViewTab::Init(void) {
    view->SetTabIntTokens(&intTokens);
    view->SetTabStringTokens(&stringTokens);
    view->SetTabLoopTokens(&loopTokens);
    view->SetTabs();    
}

void cViewTab::Left(void) {
    view->TabLeft();
}

void cViewTab::Right(void) {
    view->TabRight();
}

void cViewTab::Up(void) {
    view->TabUp();
}

void cViewTab::Down(void) {
    view->TabDown();
}

void cViewTab::Display(void) {
    if (!view)
        return;
    view->DisplayTabs();
}

/**********************************************************************
* cOsdView
**********************************************************************/
cOsdView::cOsdView(cSkinDisplayPlugin *displayPlugin) {
    this->displayPlugin = displayPlugin;
}

cOsdView::~cOsdView() {
    delete displayPlugin;
}

void cOsdView::Deactivate(bool hide) {
    if (!displayPlugin)
        return;
    displayPlugin->Deactivate(hide);
}

void cOsdView::Activate(void) {
    if (!displayPlugin)
        return;
    displayPlugin->Activate();
}

cViewElement *cOsdView::GetViewElement(int viewElementID) {
    if (!displayPlugin)
        return NULL;
    return new cViewElement(displayPlugin, viewElementID);
}

cViewGrid *cOsdView::GetViewGrid(int viewGridID) {
    if (!displayPlugin)
        return NULL;
    displayPlugin->InitGrids(viewGridID);
    return new cViewGrid(displayPlugin, viewGridID);
}

cViewTab *cOsdView::GetViewTabs(void) {
    if (!displayPlugin)
        return NULL;
    return new cViewTab(displayPlugin);
}

void cOsdView::Display(void) {
    if (!displayPlugin)
        return;
    displayPlugin->Flush();   
}
