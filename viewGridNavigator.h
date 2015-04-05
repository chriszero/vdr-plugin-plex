#ifndef CVIEWGRIDNAVIGATOR_H
#define CVIEWGRIDNAVIGATOR_H

#include <memory>
#include <vector>
#include <functional>
#include "libskindesigner/osdelements.h"

class cGridElement
{
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
	void SetPosition(double x, double y) { m_posX = x; m_posY = y; };
	void GetPosition(double &x, double &y) { x = m_posX; y = m_posY; };
	virtual void AddTokens(std::shared_ptr<cOsdElement> osdElem, bool clear = true, std::function<void(cGridElement*)> OnCached = NULL) = 0;
	int Position;
};

class cViewGridNavigator
{
protected:
	int m_rows;
	int m_columns;
	
	std::shared_ptr<cViewGrid> m_pGrid;
	cOsdView* m_pRootView;
	
	bool m_newDimensions;
	bool m_setIterator;
	std::vector<cGridElement*> m_vElements;
	std::vector<cGridElement*>::iterator m_activeElementIter;
	std::vector<cGridElement*>::iterator m_firstElementIter;
	std::vector<cGridElement*>::iterator m_lastElementIter;

	void GenerateServerElements();
	void FilterElements(int scrollOffset);
	void SetGridElementData(cGridElement *obj);
	
public:
	cViewGridNavigator(cOsdView* rootView, cViewGrid* viewGrid);
	void SetGridDimensions(int rows, int columns);
	virtual void Flush() { m_pGrid->Display(); };
	virtual void Clear() { m_pGrid->Clear(); };
	virtual void NavigateLeft();
	virtual void NavigateRight();
	virtual void NavigateUp();
	virtual void NavigateDown();
	virtual eOSState NavigateSelect() = 0;
	virtual eOSState NavigateBack() = 0;
	virtual void ReDraw(cGridElement* element);
	cGridElement* SelectedObject() { return *m_activeElementIter; }
};

#endif // CVIEWGRIDNAVIGATOR_H
