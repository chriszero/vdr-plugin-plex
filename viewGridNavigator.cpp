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

cViewGridNavigator::cViewGridNavigator(cViewGrid* viewGrid)
{
	m_columns = 2;
	m_rows = 2;
	m_newDimensions = true;
	m_setIterator = true;

	m_pGrid = std::shared_ptr<cViewGrid>(viewGrid);
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
		double width = 1.0 / m_columns;
		double height = 1.0 / m_rows;
		m_pGrid->SetGrid(element->GridElementId(), x, y, width, height);
		Flush();
		m_pRootView->Display();
	}
//
}

void cViewGridNavigator::FilterElements(int scrollOffset)
{
	int startOffset = scrollOffset;
	int endOffset = startOffset + (m_rows * m_columns);
	if(scrollOffset < 0) {
		endOffset = (m_rows * m_columns) + scrollOffset;
	}

	std::vector<cGridElement*>::iterator begin = m_firstElementIter + startOffset;
	std::vector<cGridElement*>::iterator end = m_firstElementIter + endOffset;
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
		m_pGrid->SetGrid(obj->GridElementId(), x, y, width, height);
		obj->InitFinished();
	} else {
		obj->SetPosition(x, y);
		m_pGrid->MoveGrid(obj->GridElementId(), x, y, width, height);
	}
}

void cViewGridNavigator::SetGridDimensions(int rows, int columns)
{
	m_rows = rows;
	m_columns = columns;
	m_newDimensions = true;
}

void cViewGridNavigator::NavigateDown()
{
	if(m_activeElementIter + m_columns >= m_vElements.end()) return;
	auto next = m_activeElementIter + m_columns;
	// scroll down?
	if(!(*next)->IsVisible()) {
		FilterElements(m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
}

void cViewGridNavigator::NavigateUp()
{
	if(m_activeElementIter - m_columns < m_vElements.begin()) return;
	auto next = m_activeElementIter - m_columns;
	//scroll up?
	if(!(*next)->IsVisible()) {
		FilterElements(-m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
}

void cViewGridNavigator::NavigateLeft()
{
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
}

void cViewGridNavigator::NavigateRight()
{
	auto next = m_activeElementIter + 1;
	if(next >= m_vElements.end()) next = m_vElements.end()-1;

	if(!(*next)->IsVisible()) {
		FilterElements(m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
}
