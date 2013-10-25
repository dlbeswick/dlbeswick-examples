// ------------------------------------------------------------------------------------------------
//
// UICombo
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UICombo.h"
#include "UIButton.h"
#include "UITextBox.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>

IMPLEMENT_RTTI(UIComboBase);

UIComboBase::UIComboBase() :
	m_selected(0),
	_needsSensibleListSize(false),
	_needsDisplaySizeUpdate(false)
{
	m_listBox = addChild(new UIListViewBase);
	m_listBox->onAdd = delegate(&UIComboBase::onListAdd);
	m_listBox->onSelect = delegate(&UIComboBase::onListSelect);
	m_listBox->onRemove = delegate(&UIComboBase::onListRemove);
	m_listBox->setColumns(1);
	m_listBox->setVisible(false);

	m_button = addChild(new UIButton);
	m_button->setData(new ButtonPic("dropdown", "TGA"));
	m_button->onClick = delegate(&UIComboBase::onExpand);

	m_listBox->setColour(RGBA(0,0,0,1));
	m_listBox->setInheritClipping(false);

	_needsDisplaySizeUpdate = true;
	_needsSensibleListSize = true;
}

void UIComboBase::onListAdd(const PtrGC<UIElement>&)
{
	if (m_listBox->numItems() == 1)
		select(0);
}

void UIComboBase::onListRemove()
{
	m_selected = m_listBox->selected();
}

void UIComboBase::select(int idx)
{
	if (idx == -1)
	{
		m_selected = 0;
		return;
	}

	idx = std::min((uint)idx, list().numItems() - 1);

	m_listBox->select(idx);
	m_selected = list().selected();
}

void UIComboBase::update(float delta)
{
	Super::update(delta);
}

void UIComboBase::onDraw()
{
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, m_displayAreaSize.y);
	D3DPaint().quad2D(0.001f, 0.001f, m_displayAreaSize.x, m_displayAreaSize.y);
	D3DPaint().draw();

	if (m_selected)
	{
		Point2 minClip;
		Point2 maxClip;

		minClip.x = std::max(screenPos().x, this->minClip().x);
		minClip.y = std::max(screenPos().y, this->minClip().y);
		maxClip.x = std::min(screenPos().x + m_displayAreaSize.x, this->maxClip().x);
		maxClip.y = std::min(screenPos().y + m_displayAreaSize.y, this->maxClip().y);

		m_selected->drawAt(screenPos() + Point2(0.001f, 0.001f), minClip, maxClip);
	}
}

void UIComboBase::onExpand()
{
	m_listBox->setVisible(!m_listBox->visible());
	
	if (m_listBox->visible())
	{
		if (_needsSensibleListSize)
			sizeListSensibly();

		adjustListPosAndSize();
		bringToFront();
	}
}

void UIComboBase::adjustListPosAndSize()
{
	bool isOnASide = false;
	float dlgRootTop = dlgRoot()->clientToScreen(Point2(0,0)).y;
	float dlgRootBottom = dlgRoot()->clientToScreen(Point2(0,dlgRoot()->size().y)).y;

	m_listBox->setSize(Point2(m_fullSize.x, m_fullSize.y));

	Point2 newListPos = Point2(0, m_displayAreaSize.y);

	// position listbox on top of combo if it extends past the bottom of the screen
	if (clientToScreen(newListPos + Point2(0, m_listBox->size().y)).y > dlgRootBottom)
		newListPos.y = -m_listBox->size().y;

	// if listbox now extends past the top of the screen, then try placing it to the right.
	if (clientToScreen(newListPos).y < dlgRootTop)
	{
		newListPos.x = size().x;
		isOnASide = true;
	}

	// if listbox now extends past the right of the screen, then try placing it to the left.
	if (clientToScreen(newListPos).x > dlgRoot()->clientToScreen(Point2(dlgRoot()->size().x, 0)).x)
	{
		newListPos.x = 0 - m_listBox->size().x;
		isOnASide = true;
	}

	// if listbox now extends past the left of the screen, then finally put it on either the top or the bottom,
	// depending on which has more space, then try and fix the sizing so it remains within bounds.
	if (clientToScreen(newListPos).x < dlgRoot()->clientToScreen(Point2(0, 0)).x)
	{
		isOnASide = false;

		if (dlgRootBottom - clientToScreen(pos()).y > dlgRootTop - clientToScreen(pos()).y)
		{
			newListPos = Point2(0, m_displayAreaSize.y);
			m_listBox->setSize(Point2(m_fullSize.x, std::min(m_fullSize.y, dlgRootBottom - newListPos.y)));
		}
		else
		{
			m_listBox->setSize(Point2(m_fullSize.x, std::min(m_fullSize.y, clientToScreen(pos()).y - dlgRootTop)));
			newListPos = Point2(0, -m_listBox->size().y);
		}
	}

	// If the listbox was placed on a side, then make sure it fits in the vertical screen bounds.
	if (isOnASide)
	{
		newListPos.y = std::min(0.0f, dlgRootBottom - clientToScreen(newListPos).y + m_listBox->size().y);
	}

	m_listBox->setPos(newListPos);
}

void UIComboBase::add(UIElement* i)
{
	m_listBox->add(i);
}

void UIComboBase::onListSelect(const PtrGC<UIElement>&)
{
	m_listBox->setVisible(false);
	UIElement::setSize(m_displayAreaSize);
	m_selected = m_listBox->selected();

	onSelect(m_selected);
}

void UIComboBase::remove(int idx)
{
	m_listBox->remove(idx);
}

bool UIComboBase::onMouseDown(const Point2 &p, int button)
{
	onExpand();

	return true;
}

void UIComboBase::sizeListSensibly()
{
	_needsSensibleListSize = false;
	m_fullSize = m_listBox->sizeOfAllItems();
	clamp(m_fullSize.x, m_displayAreaSize.x, std::max(m_displayAreaSize.x, 0.4f));
	clamp(m_fullSize.y, 0.0f, 0.3f);
}

void UIComboBase::setSize(const Point2& reqSize)
{
	_needsSensibleListSize = false;

	m_fullSize = reqSize;

	updateDisplayAreaSize();
}

void UIComboBase::updateDisplayAreaSize()
{
	Point2 itemSize;

	if (m_selected)
		itemSize = m_selected->size();
	else if (list().numItems())
		itemSize = m_listBox->sizeOfAllItems();
	else
		itemSize = Point2(0.1f, font().height());

	m_displayAreaSize(m_fullSize.x, itemSize.y);

	m_displayAreaSize.x = std::max(m_displayAreaSize.x, 0.05f);

	// buffer space
	m_displayAreaSize.y += 0.002f;

	Point2 buttonSize(m_displayAreaSize.y, m_displayAreaSize.y);
	m_button->setSize(buttonSize);
	m_button->setPos(Point2(m_displayAreaSize.x, 0));

	m_displayAreaSize.x += m_button->size().x;
	UIElement::setSize(m_displayAreaSize);
}

// overridden -- when the list box is expanded, errant clicks should close it
bool UIComboBase::isMouseOver(const Point2& screenPos)
{
	return m_listBox->visible() || UIElement::isMouseOver(screenPos);
}

void UIComboBase::_onFontChange()
{
	Super::_onFontChange();
	updateDisplayAreaSize();
}
