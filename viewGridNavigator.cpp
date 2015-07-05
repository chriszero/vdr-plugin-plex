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

cViewGridNavigator::cViewGridNavigator(std::shared_ptr<skindesignerapi::cOsdView> rootView)
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

	m_startIndex += scrollOffset;
	if(m_startIndex < 0) m_startIndex = 0;

	m_endIndex = m_startIndex + (m_rows * m_columns);
	if(m_endIndex > m_vElements.size()) m_endIndex = m_vElements.size();
	
	if(scrollOffset > 0 && m_startIndex >= m_endIndex) {
		// allign elements
		int delta = m_vElements.size() % m_columns;
		delta = m_rows * m_columns - m_columns + delta;
		m_startIndex = m_endIndex - delta;
	}
	
	unsigned int i = 0;
	int pos = 0;
	for(auto it = m_vElements.begin(); it != m_vElements.end(); ++it) {

		if(i >= m_startIndex && i < m_endIndex) {
			cGridElement *elem = *it;
			elem->Position = pos++;
			SetGridElementData(elem);

			if(m_setIterator) {
				m_activeElementIter = it;
				m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), true);
				m_setIterator = false;
			}
		} else {
			m_pGrid->Delete((*it)->GridElementId());
			(*it)->Dirty();
			(*it)->Position = -1;
			(*it)->SetPosition(-1,-1);
			// Remove Queued Downloads
			cPictureCache::GetInstance().Remove(*it);
		}
		i++;
	}

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
	if (m_setIterator ) return false;
	auto next = m_activeElementIter + m_columns;

	bool scrollallaround = false;
	if (next >= m_vElements.end() && Config::GetInstance().ScrollAllAround) {
		next = m_vElements.begin();
		scrollallaround = true;
	} else if(next >= m_vElements.end()) {
		next = m_activeElementIter; // stay at current element //m_vElements.end()-1;
		return false;
	}

	// scroll down?
	if(!(*next)->IsVisible()) {
		int scrolloffset = m_columns;
		if(Config::GetInstance().ScrollByPage) {
			scrolloffset = m_columns*m_rows;
		}
		if(scrollallaround) 	
			scrolloffset = -((*m_activeElementIter)->AbsolutePosition);
			
		FilterElements(scrolloffset);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}

bool cViewGridNavigator::NavigateUp()
{
	if (m_setIterator) return false;
	auto next = m_activeElementIter - m_columns;

	bool scrollallaround = false;
	if (next < m_vElements.begin() && Config::GetInstance().ScrollAllAround) {
		next = m_vElements.end() - 1;
		scrollallaround = true;
	} else if(next < m_vElements.begin()) {
		next = m_activeElementIter; // stay at current element //m_vElements.end()-1;
		return false;
	}

	//scroll up?
	if(!(*next)->IsVisible()) {
		int scrolloffset = -m_columns;
		if(Config::GetInstance().ScrollByPage) {
			scrolloffset = -m_columns*m_rows;
		}
		if(scrollallaround)
			scrolloffset = m_vElements.size();
		
		FilterElements(scrolloffset);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}

bool cViewGridNavigator::NavigateLeft()
{
	if (m_setIterator || m_activeElementIter == m_vElements.begin()) return false;
	auto next = m_activeElementIter - 1;
	if(next < m_vElements.begin()) next = m_vElements.begin();

	if(!(*next)->IsVisible()) {
		auto temp = m_activeElementIter;
		if(Config::GetInstance().ScrollByPage)
			FilterElements(-m_columns*m_rows);
		else
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
	if (m_setIterator || m_activeElementIter == m_vElements.end() - 1) return false;
	auto next = m_activeElementIter + 1;
	if(next >= m_vElements.end()) next = m_vElements.end()-1;

	if(!(*next)->IsVisible()) {
		if(Config::GetInstance().ScrollByPage)
			FilterElements(m_columns*m_rows);
		else
			FilterElements(m_columns);
	}

	m_pGrid->SetCurrent((*m_activeElementIter)->GridElementId(), false);
	m_pGrid->SetCurrent((*next)->GridElementId(), true);
	m_activeElementIter = next;
	return true;
}
