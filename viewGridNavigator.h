#ifndef CVIEWGRIDNAVIGATOR_H
#define CVIEWGRIDNAVIGATOR_H

#include <memory>
#include <vector>
#include "libskindesigner/osdelements.h"

class cGridElement
{
private:
	bool m_bInit;
	unsigned int m_iGridId;
	static unsigned int AbsoluteGridIdCounter;	

public:
	cGridElement();
	
	bool IsNew() { return m_bInit; };
    void Dirty() { m_bInit = true; };
    void InitFinished() { m_bInit = false; };
	unsigned int GridElementId() { return m_iGridId; }
	bool IsVisible() { return Position > -1; }
	virtual void AddTokens(std::shared_ptr<cViewGrid> grid) = 0;
	int Position;
};

class cViewGridNavigator
{
protected:	
	int m_rows;
	int m_columns;
	
	std::shared_ptr<cViewGrid> m_pGrid;
	
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
	cViewGridNavigator(cViewGrid* viewGrid);
	void SetGridDimensions(int rows, int columns);
	void Flush() { m_pGrid->Display(); };
	void DrawGrid() { m_pGrid->Display(); }
	virtual void NavigateLeft();
	virtual void NavigateRight();
	virtual void NavigateUp();
	virtual void NavigateDown();
	virtual eOSState NavigateSelect() = 0;
	virtual eOSState NavigateBack() = 0;
	cGridElement* SelectedObject() { return *m_activeElementIter; }
	
};

#endif // CVIEWGRIDNAVIGATOR_H
