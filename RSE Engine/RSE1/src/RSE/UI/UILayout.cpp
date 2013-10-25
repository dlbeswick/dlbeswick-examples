// ------------------------------------------------------------------------------------------------
//
// UILayout
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UILayout.h"

IMPLEMENT_RTTI(UILayout);

UILayout::UILayout() :
	bindToParent(true),
	m_needsRecalc(true)
{
	if (parent())
		setParent(parent());
}

void UILayout::update(float delta)
{
	Super::update(delta);
	if (m_needsRecalc)
	{
		forceRecalc();
	}
}

void UILayout::setParent(const PtrGC<UIElement>& pParent)
{
	Super::setParent(pParent);
	if (parent())
	{
		parent()->onSizeChanged.add(*this, &UILayout::onParentSizeChanged);
		onParentSizeChanged();
	}
}

void UILayout::onParentSizeChanged()
{
	if (bindToParent && parent())
	{
		setSize(parent()->size());
	}
}

UIElement* UILayout::_addChild(const PtrGC<UIElement>& child)
{
	Super::_addChild(child);
	m_needsRecalc = true;
	m_layoutChildren.add(child);

	return child.ptr();
}

UIElement* UILayout::addChildExplicit(const PtrGC<UIElement>& child)
{
	return Super::_addChild(child);
}

void UILayout::removeChild(const PtrGC<UIElement>& child)
{
	Super::removeChild(child);
	m_layoutChildren.remove(child);
	m_needsRecalc = true;
}

void UILayout::onFontChange()
{
	m_needsRecalc = true;
}

void UILayout::forceRecalc()
{
	m_layoutChildren.flush();
	recalc();
	m_needsRecalc = false;
}

void UILayout::setSize(const Point2& s)
{
	m_needsRecalc = true;
	Super::setSize(s);
}

void UILayout::onDraw()
{
	if (m_needsRecalc)
	{
		forceRecalc();
	}
}

///////////////////////////////////////

IMPLEMENT_RTTI(UILayoutFill);

void UILayoutFill::recalc()
{
	setSize(parent()->size());

	for (ElementList::iterator i = m_layoutChildren.begin(); i != m_layoutChildren.end(); ++i)
	{
		UIElement& e = **i;
		e.setSize(size());
	}
}

///////////////////////////////////////

IMPLEMENT_RTTI(UILayoutGrid);

void UILayoutGrid::recalc()
{
	Point2 largest(-FLT_MAX, -FLT_MAX);
	// find largest x and y
	for (ElementList::iterator i = m_layoutChildren.begin(); i != m_layoutChildren.end(); ++i)
	{
		UIElement& e = **i;
		if (e.size().x > largest.x)
			largest.x = e.size().x;

		if (e.size().y > largest.y)
			largest.y = e.size().y;
	}

	int row = 0;
	int col = 0;

	// arrange children
	for (ElementList::iterator i = m_layoutChildren.begin(); i != m_layoutChildren.end(); ++i)
	{
		UIElement& e = **i;

		if (m_bFillMethod)
			e.setPos(Point2(largest.x * (float)col, largest.y * (float)row));
		else
			e.setPos(Point2(size().x - e.size().x - largest.x * (float)col, largest.y * (float)row));

		++col;
		if (col >= m_cols)
		{
			++row;
			col = 0;
		}
	}
}

///////////////////////////////////////

IMPLEMENT_RTTI(UILayoutTable);

UILayoutTable::UILayoutTable()
{
	setCols(1, true);
	setRowSpacing(0.0005f);
	cellAlign = ALIGN_CENTER;
	cellVAlign = VALIGN_CENTER;
}

void UILayoutTable::setCols(int cols, bool autoSize)
{
	m_cols.resize(cols);
	if (autoSize)
	{
		for (int i = 0; i < cols; ++i)
			setCol(i, 1.0f / cols);
	}
}

void UILayoutTable::setCol(int idx, float width)
{
	if (idx >= (int)m_cols.size())
		throwf(std::string("UILayoutTable::setCol -- index ") + idx + std::string(" out of range ") + m_cols.size());

	m_cols[idx] = width;
}

void UILayoutTable::recalc()
{
	float posY = 0;
	float largestY = 0;
	int lastLayoutElement = 0;
	int currentLayoutElement = 0;
	int col = 0;

	// trim unneeded layout elements
	if (m_layoutChildren.size() < m_layoutElements.size())
	{
		for (uint i = m_layoutChildren.size(); i < m_layoutElements.size(); ++i)
		{
			PtrGC<UIElement>(m_layoutElements[i]).destroy();
		}

		m_layoutElements.resize(m_layoutChildren.size());
	}

	// arrange children
	for (ElementList::iterator i = m_layoutChildren.begin(); i != m_layoutChildren.end(); ++i)
	{
		UIElement& e = **i;

		// keep track of largest y size of row
		largestY = std::max(e.size().y, largestY);

		// create layout element if needed
		if ((int)m_layoutElements.size() <= currentLayoutElement)
		{
			m_layoutElements.resize(currentLayoutElement + 1);
			m_layoutElements[currentLayoutElement] = new UIContainer;
			addChildExplicit(m_layoutElements[currentLayoutElement]);
		}

		m_layoutElements[currentLayoutElement]->addChild(&e);

		++currentLayoutElement;
		++col;
		if (col >= (int)m_cols.size())
		{
			// go from lastLayoutElement to currentLayoutElement positioning and resizing layout elements
			col = 0;
			float posX = 0;
			for (uint idx = lastLayoutElement; (int)idx < currentLayoutElement; ++idx)
			{
				float colWidth = m_cols[col] * size().x;
				m_layoutElements[idx]->setSize(Point2(colWidth, largestY));
				m_layoutElements[idx]->setPos(Point2(posX, posY));

				UIElement* child = m_layoutElements[idx]->children()[0].ptr();
				if (child)
					child->align(cellAlign, cellVAlign, m_layoutElements[idx]);
				posX += colWidth;
				++col;
			}

			lastLayoutElement = currentLayoutElement;

			// next row
			posY += largestY + m_rowSpacing;
			col = 0;
			largestY = 0;
		}
	}

	setSize(Point2(size().x, std::max(0.0f, posY - m_rowSpacing)));
}

void UILayoutTable::setRowSpacing(float y)
{
	m_rowSpacing = y;
}
