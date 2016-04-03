#ifndef cViewGridNAVIGATOR_H
#define cViewGridNAVIGATOR_H

#include <memory>
#include <vector>
#include <functional>
#include  <libskindesignerapi/osdelements.h>

class cGridElement {
private:
    bool m_bInit;
    double m_posX;
    double m_posY;
    static unsigned int AbsoluteGridIdCounter;

protected:
    unsigned int m_iGridId;

public:
    cGridElement();

    bool IsNew() { return m_bInit; };

    void Dirty() { m_bInit = true; };

    void InitFinished() { m_bInit = false; };

    unsigned int GridElementId() { return m_iGridId; }

    bool IsVisible() { return Position > -1; }

    void SetPosition(double x, double y) {
        m_posX = x;
        m_posY = y;
    };

    void GetPosition(double &x, double &y) {
        x = m_posX;
        y = m_posY;
    };

    virtual void AddTokens(std::shared_ptr<skindesignerapi::cOsdElement> osdElem, bool clear = true,
                           std::function<void(cGridElement *)> OnCached = NULL) = 0;

    int Position;
    int AbsolutePosition;
};

class cViewGridNavigator {
protected:
    int m_rows;
    int m_columns;
    bool m_bHidden;
    bool m_bEnableRedraw;

    std::shared_ptr<skindesignerapi::cOsdView> m_pRootView;
    std::shared_ptr<skindesignerapi::cViewGrid> m_pGrid;
    std::shared_ptr<skindesignerapi::cViewElement> m_pScrollbar = nullptr;

    bool m_newDimensions;
    bool m_setIterator;
    std::vector<cGridElement *> m_vElements;
    std::vector<cGridElement *>::iterator m_activeElementIter;
    int m_startIndex;
    int m_endIndex;

    void FilterElements(int scrollOffset);

    void SetGridElementData(cGridElement *obj);

    void SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid> grid);

public:
    cViewGridNavigator(std::shared_ptr<skindesignerapi::cOsdView> rootView,
                       std::shared_ptr<skindesignerapi::cViewElement> pScrollbar);

    void SetGridDimensions(int rows, int columns);

    virtual void Flush() {
        m_pGrid->Display();
        m_pRootView->Display();
    };

    virtual void Clear() = 0;

    virtual bool NavigateLeft();

    virtual bool NavigateRight();

    virtual bool NavigateUp();

    virtual bool NavigateDown();

    virtual eOSState NavigateSelect() = 0;

    virtual eOSState NavigateBack() = 0;

    virtual void ReDraw(cGridElement *element);

    cGridElement *SelectedObject();

    virtual void Deactivate(bool hide);

    virtual void Activate();

    void DrawScrollbar();
};

#endif // cViewGridNAVIGATOR_H
