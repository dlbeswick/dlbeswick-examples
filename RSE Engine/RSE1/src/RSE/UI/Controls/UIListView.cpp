// ------------------------------------------------------------------------------------------------
//
// UIListView
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIListView.h"
#include "ScrollingContainer.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include "UI/UILayout.h"
#include "Standard/DXInput.h"

static const float SPACING = 0.005f;

IMPLEMENT_RTTI(UIListViewBase);
REGISTER_RTTI_NAME(UIListView, "List View");

// construct
UIListViewBase::UIListViewBase() :
	m_columns(1),
	m_rows(0),
	m_selected(-1)
{
	_container = addChild(new UILayoutFill)->addChild(new ScrollingContainer);
}

// add
void UIListViewBase::add(UIElement* item)
{
	m_bRecalcLayout = true;
	
	m_items.push_back(item);
	_container->addChild(item);

	if (numItems() == 1)
		m_selected = 0;

	onAdd(item);
}

void UIListViewBase::remove(int idx)
{
	m_bRecalcLayout = true;

	PtrGC<UIElement> item = *(m_items.begin() + idx);
	removeChild(item);
	item.destroy();
	m_items.erase(m_items.begin() + idx);

	if (m_items.empty())
	{
		select(-1);
	}
	else if (m_selected >= (int)m_items.size())
	{
		select(m_items.size()-1);
	}

	onRemove();
}

void UIListViewBase::select(int idx)
{
	clamp(idx, -1, (int)m_items.size()-1);
	m_selected = idx;
}

// onDraw
void UIListViewBase::onDraw()
{
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();

	if (m_selected != -1)
	{
		Point2 itemPos(screenToClient(_container->clientArea().clientToScreen(m_items[m_selected]->pos())));
		Point2 itemSize = m_cellSize;

		D3DPaint().setFill(RGBA(1, 1, 1, 0.25f));
		D3DPaint().quad2D(itemPos.x, itemPos.y, itemPos.x + itemSize.x, itemPos.y + itemSize.y);
		D3DPaint().draw();
	}

	if (m_bRecalcLayout)
		calcLayout();
}

// calcLayout
void UIListViewBase::calcLayout()
{
	m_bRecalcLayout = false;

	// find largest dimensions
	Point2 largest(-FLT_MAX, -FLT_MAX);
	for (uint i = 0; i < m_items.size(); ++i)
	{
		UIElement& m = *m_items[i];
		largest.x = std::max(largest.x, m.size().x);
		largest.y = std::max(largest.y, m.size().y);
	}

	int rows = m_rows;
	int cols = m_columns;

	if (m_rows == 0)
		std::swap(cols, rows);

	Items::iterator it = m_items.begin();

	Point2 p;

	for (int i = 0; it != m_items.end(); ++i)
	{
		if (m_rows == 0)
			p(0, (largest.y + SPACING) * i);
		else
			p((largest.x + SPACING) * i, 0);

		for (int j = 0; j < rows && it != m_items.end(); ++j)
		{
			(*it)->setPos(p);
			++it;

			if (m_rows == 0)
				p.x += largest.x + SPACING;
			else
				p.y += largest.y + SPACING;
		}
	}

	// record space taken up by items
	m_cellSize(largest.x + SPACING, largest.y + SPACING);
	m_allItemsSize = p;

	m_realCols = (int)(m_allItemsSize.x / m_cellSize.x);
	m_realRows = (int)(m_allItemsSize.y / m_cellSize.y);

	_container->clientArea().fitToChildren();
}

Point2 UIListViewBase::sizeOfAllItems()
{
	calcLayout();

	Point2 min(FLT_MAX, FLT_MAX);
	Point2 max(-FLT_MAX, -FLT_MAX);

	getRealScreenExtents(min, max);
	return max - min;
}

Point2 UIListViewBase::calcItemSize(Point2& largest)
{
	// ensure that column space is used fully
	// i don't know what this was for, find out
/*	if (m_rows == 0)
		largest.x = std::max(largest.x, (itemAreaSize().x - (m_columns-1) * SPACING) / m_columns);
	else
		largest.y = std::max(largest.y, (itemAreaSize().y - (m_rows-1) * SPACING) / m_rows);*/

	if (m_rows == 0)
	{
		return Point2(largest.x * m_columns + (SPACING * (m_columns - 1)), 
						largest.y * (m_items.size() / m_columns) + SPACING * ((m_items.size() / m_columns) - 1));
	}
	else
	{
		return Point2(largest.x * (m_items.size() / m_rows) + SPACING * ((m_items.size() / m_rows) - 1), 
						largest.y * m_rows + (SPACING * (m_rows - 1)));
	}
}

UIElement* UIListViewBase::item(int idx)
{
	if (idx >= 0 && idx < (int)m_items.size())
		return m_items[idx];
	else
		return 0;
}

bool UIListViewBase::mouseDown(const Point2 &p, int button)
{
	if (button == 0)
	{
		int rows = m_realRows;

		Point2 relPos = _container->clientArea().screenToClient(clientToScreen(p));
		int selected = (int)((relPos.y / m_cellSize.y) + (rows * (int)(relPos.x / m_cellSize.x)));
		if (selected >= 0 && selected < (int)m_items.size())
		{
			select(selected);
			onSelect(m_items[selected]);
		}
	}

	return true;
}

void UIListViewBase::sort()
{
}

bool UIListViewBase::mouseDouble(const Point2 &p, int button)
{
	mouseDown(p, button);
	if (m_selected >= 0 && m_selected < (int)m_items.size())
	{
		onDoubleClick(m_items[m_selected]);
	}
	return true;
}

// ListText
// Text Items
ListText::ListText(const std::string &text, const std::string &data) : 
	m_text(text),
	m_data(data)
{
	_onFontChange();
};

void ListText::_onFontChange()
{
	setSize(Point2(font().stringWidth(m_text), font().height()));
}

void ListText::onDraw()
{
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();
	//font().setColour(colour());
	font().write(D3DPaint(), m_text, 0, 0);
	D3DPaint().draw();
}
