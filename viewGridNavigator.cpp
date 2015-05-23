#include "viewGridNavigator.h"
#include <iostream>
#include "plexSdOsd.h"
#include "pictureCache.h"

unsigned int cGridElement::AbsoluteGridIdCounter = 0;

cGridElement::cGridElement()
{
	m_iGridId = AbsoluteGridIdCounter++;
	Position = -1;
	m_bInit = true;
}

cViewGridNavigator::cViewGridNavigator(skindesignerapi::cOsdView* rootView)
{
	m_columns = 2;
	m_rows = 2;
	m_newDimensions = true;
	m_setIterator = true;

	m_pGrid = NULL;
	m_pRootView = rootView;
}

void cViewGridNavigator::SetViewGrid(std::shared_ptr<skindesignerapi::cViewGrid> grid)
{
	if (grid) {
		if(m_pGrid) m_pGrid->Clear();
		m_pGrid = std::shared_ptr<skindesignerapi::cViewGrid>(grid);
	}
}

void cViewGridNavigator::ReDraw(cGridElement* element)
{
	if(element) {
		cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
		if (!element->IsVisible()) {
			std::cout << "ReDraw element not visible" << std::endl;
			return;
		}
		double x, y;
		element->GetPosition(x, y);
		element->AddTokens(m_pGrid);
		m_pGrid->AddIntToken("columns", m_columns);
		m_pGrid->AddIntToken("rows", m_rows);
		double width = 1.0 / m_columns;
		double height = 1.0 / m_rows;
		m_pGrid->SetGrid(element->GridElementId(), x, y, width, height);
		Flush();
	}
}

void cViewGridNavigator::FilterElements(int scrollOffset)
{
	if(m_vElements.size() == 0) return;

	int startOffset = scrollOffset;
	int endOffset = startOffset + (m_rows * m_columns);
	if(scrollOffset < 0) {
		endOffset = (m_rows * m_columns) + scrollOffset;
	}

	//remove non visible elements from grid
	if(scrollOffset !=0 ) {
		auto startIt = m_firstElementIter;
		auto endIt = m_firstElementIter + scrollOffset;
		if(scrollOffset < 0) {
			startIt = m_lastElementIter + scrollOffset;
			endIt = m_lastElementIter;
		}
		if(startIt < m_vElements.begin()) startIt = m_vElements.begin();
		if(endIt > m_vElements.end()) endIt = m_vElements.end();

		for(std::vector<cGridElement*>::iterator it = startIt; it != endIt; ++it) {
			m_pGrid->Delete((*it)->GridElementId());
			(*it)->Dirty();
			(*it)->Position = -1;
			(*it)->SetPosition(-1,-1);
			// Remove Queued Downloads
			cPictureCache::GetInstance().Remove(*it);
		}
	}

	std::vector<cGridElement*>::iterator begin = m_firstElementIter + startOffset;
	std::vector<cGridElement*>::iterator end = m_firstElementIter + endOffset;
	if(begin < m_vElements.begin()) begin = m_vElements.begin();
	if(end > m_vElements.end()) end = m_vElements.end();

	int pos = 0;
	for(std::vector<cGridElement*>::iterator it = begin; it != end; ++it) {
		cGridElement *elem = *it;
		elem->Position = pos++;
		SetGridElementData(elem);
	}

	if(m_setIterator) {
		m_activeElementIter = begin;
		m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), true);
		m_setIterator = false;
	}

	m_firstElementIter = begin;
	m_lastElementIter = end;
	m_newDimensions = false;
}

void cViewGridNavigator::SetGridElementData(cGridElement *obj)
{
	// calculate position
	double row, column, x, y, height, width;

	row = floor(obj->Position / m_columns);
	column = obj->Position - (row * m_columns);
	width = 1.0 / m_columns;
	height = 1.0 / m_rows;
	x = width * column;
	y= height * row;

	//std::cout << "ID: " << obj->GridElementId() << "\tPos: " << obj->Position << "\t\tx: " << x << "\t\ty: " << y << "\t\twi: " << width << "\t\thei: " <<  height << "\tCol: " << column << "\tRow: " << row << std::endl;
	cMutexLock MutexLock(&cPlexSdOsd::RedrawMutex);
	if(obj->IsNew() || m_newDimensions) {
		// fill data
		obj->SetPosition(x, y);
		obj->AddTokens(m_pGrid, true, std::bind(&cViewGridNavigator::ReDraw, this, std::placeholders::_1));
		// set GridDimensions
		m_pGrid->AddIntToken("columns", m_columns);
		m_pGrid->AddIntToken("rows", m_rows);
		m_pGrid->AddIntToken("position", obj->AbsolutePosition);
		m_pGrid->AddIntToken("totalcount", m_vElements.size());
		m_pGrid->SetGrid(obj->GridElementId(), x, y, width, height);
		obj->InitFinished();
	} else {
		obj->SetPosition(x, y);
		m_pGrid->MoveGrid(obj->GridElementId(), x, y, width, height);
	}
}

cGridElement* cViewGridNavigator::SelectedObject()
{
	if(!m_setIterator) 
		return *m_activeElementIter;
	return NULL;
}

void cViewGridNavigator::SetGridDimensions(int rows, int columns)
{
	m_rows = rows;
	m_columns = columns;
	m_newDimensions = true;
}

bool cViewGridNavigator::NavigateDown()
{
	if (m_activeElementIter == m_vElements.end() - 1) return false;
	auto next = m_activeElementIter + m_columns;
	if(next >= m_vElements.end()) next = m_vElements.end()-1;

	// scroll down?
	if(!(*next)->IsVisible()) {
		FilterElements(m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}

bool cViewGridNavigator::NavigateUp()
{
	if (m_activeElementIter == m_vElements.begin()) return false;
	auto next = m_activeElementIter - m_columns;
	if(next < m_vElements.begin()) next = m_vElements.begin();

	//scroll up?
	if(!(*next)->IsVisible()) {
		FilterElements(-m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}

bool cViewGridNavigator::NavigateLeft()
{
	if (m_activeElementIter == m_vElements.begin()) return false;
	auto next = m_activeElementIter - 1;
	if(next < m_vElements.begin()) next = m_vElements.begin();

	if(!(*next)->IsVisible()) {
		auto temp = m_activeElementIter;
		FilterElements(-m_columns);
		m_activeElementIter = temp;
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}

bool cViewGridNavigator::NavigateRight()
{
	if (m_activeElementIter == m_vElements.end() - 1) return false;
	auto next = m_activeElementIter + 1;
	if(next >= m_vElements.end()) next = m_vElements.end()-1;

	if(!(*next)->IsVisible()) {
		FilterElements(m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}
