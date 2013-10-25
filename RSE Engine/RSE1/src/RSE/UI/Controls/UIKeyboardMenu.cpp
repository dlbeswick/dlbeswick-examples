// ------------------------------------------------------------------------------------------------
//
// UIKeyboardMenu
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIKeyboardMenu.h"
#include "UI/Controls/UIPic.h"

IMPLEMENT_RTTI(UIKeyboardMenuBase)
IMPLEMENT_RTTI(UIKeyboardMenu)

UIKeyboardMenuBase::UIKeyboardMenuBase()
{
	prevKey = VK_UP;
	nextKey = VK_DOWN;
	// fix, fill to parent
	setSize(Point2(100.0f, 100.0f));
}

void UIKeyboardMenuBase::update(float delta)
{
	Super::update(delta);

	if (!m_selection)
		select(next(m_selection));
}

PtrGC<UIElement> UIKeyboardMenuBase::prev(const PtrGC<UIElement>& p)
{
	ElementList& c = m_items;
	c.flush();
	if (c.empty())
		return 0;

	if (!p)
		return c[c.size() - 1];

	for (int i = (int)c.size() - 1; i > -1; --i)
	{
		if (c[i] == this)
			continue;

		if (c[i] == p)
		{
			if (i == 0)
				return c[c.size() - 1];
			else
				return c[i - 1];
		}
	}
	
	return p;
}

PtrGC<UIElement> UIKeyboardMenuBase::next(const PtrGC<UIElement>& p)
{
	ElementList& c = m_items;
	c.flush();
	if (c.empty())
		return 0;

	if (!p)
		return c[0];

	for (uint i = 0; i < c.size(); ++i)
	{
		if (c[i] == this)
			continue;

		if (c[i] == p)
		{
			if (i == c.size() - 1)
				return c[0];
			else
				return c[i + 1];
		}
	}

	return p;
}

void UIKeyboardMenuBase::select(const PtrGC<UIElement>& p)
{
	// fix
	if (p == this)
		return;

	m_keyboardFocus = keyboardFocusFor(p);
	m_selection = p;
	onSelected();
}

PtrGC<UIElement> UIKeyboardMenuBase::keyboardFocusFor(const PtrGC<UIElement>& p)
{
	if (!p)
		return 0;

	if (p->canFocus())
	{
		return p;
	}

	for (ElementList::iterator i = p->children().begin(); i != p->children().end(); ++i)
	{
		PtrGC<UIElement> what;
		what = keyboardFocusFor(*i);
		if (what)
			return what;
	}

	return 0;
}

void UIKeyboardMenuBase::keyDown(int key)
{
	Super::keyDown(key);
	if (!usesKey(key))
	{
		if (m_keyboardFocus)
			m_keyboardFocus->keyDown(key);
	}
	else
	{
		if (key == prevKey)
			select(prev(m_selection));
		else if (key == nextKey)
			select(next(m_selection));
	}
}

void UIKeyboardMenuBase::keyPressed()
{
	Super::keyPressed();
	if (m_keyboardFocus)
		m_keyboardFocus->keyPressed();
}

void UIKeyboardMenuBase::keyUp(int key)
{
	Super::keyUp(key);
	if (!usesKey(key) && m_keyboardFocus)
	{
		if (m_keyboardFocus)
			m_keyboardFocus->keyUp(key);
	}
}

void UIKeyboardMenuBase::keyChar(int key)
{
	Super::keyChar(key);
	if (!usesKey(key) && m_keyboardFocus)
	{
		if (m_keyboardFocus)
			m_keyboardFocus->keyChar(key);
	}
}

bool UIKeyboardMenuBase::usesKey(int key)
{
	return key == prevKey
		|| key == nextKey
		;
}

void UIKeyboardMenuBase::add(const PtrGC<UIElement>& e)
{
	m_items.add(e);
}

////

// style
UISTYLESTREAM(UIKeyboardMenu)
	STREAMVAR(leftArrowMaterialName);
	STREAMVAR(rightArrowMaterialName);
	STREAMVARDEFAULT(leftArrowSize, Point2::ZERO);
	STREAMVARDEFAULT(rightArrowSize, Point2::ZERO);
UISTYLESTREAMEND

UIKeyboardMenu::UIKeyboardMenu()
{
}

void UIKeyboardMenu::construct()
{
	Super::construct();
	m_arrows[0] = addChild(new UIPic(material(style().rightArrowMaterialName)));
	m_arrows[1] = addChild(new UIPic(material(style().leftArrowMaterialName)));
	setArrowSize(style().leftArrowSize, style().rightArrowSize);
}

void UIKeyboardMenu::onSelected()
{
	std::for_each(m_arrows, m_arrows + 2, std::bind2nd(std::mem_fun(&UIKeyboardMenu::setVisible), m_selection != 0));

	if (!m_selection)
		return;

	Point2 selPos = screenToClient(m_selection->screenPos());

	m_arrows[0]->setPos(Point2(selPos.x - m_arrows[0]->size().x * 1.1f, selPos.y));
	m_arrows[0]->align(ALIGN_NONE, VALIGN_CENTER, m_selection);
	m_arrows[1]->setPos(Point2(selPos.x + m_selection->size().x + m_arrows[1]->size().x * 0.1f, selPos.y));
	m_arrows[1]->align(ALIGN_NONE, VALIGN_CENTER, m_selection);
}

void UIKeyboardMenu::setArrowSize(const Point2& sizeLeft, const Point2& sizeRight)
{
	if (sizeLeft != Point2::ZERO)
		m_arrows[0]->setSize(sizeLeft);

	if (sizeRight != Point2::ZERO)
		m_arrows[1]->setSize(sizeRight);
}

