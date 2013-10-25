// ---------------------------------------------------------------------------------------------------------
// 
// UIMenu
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include <Standard/FileHelp.h>
#include "UIMenu.h"
#include "UI/DialogMgr.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include "Render/SFont.h"

static float BORDER_HEIGHT = 0.005f;
static float BORDER_WIDTH = BORDER_HEIGHT;
static float BORDER_HEIGHT2 = BORDER_HEIGHT * 2;
static float BORDER_WIDTH2 = BORDER_WIDTH * 2;

static float EXPAND_SIZEX = 0.015f;
static float EXPAND_SIZEYPERC = 0.5f;
static float EXPAND_GAP	= 0.0025f;

static float CHECK_SIZEYPERC = 0.75f;
static float CHECK_GAP	= 0.0025f;

// Menu Items

void MenuItem::setChecked(bool b)
{
	m_bChecked = b;
}


Point2 MenuItem::requestedSize()
{
	Point2 size = onCalcSize();

	if (m_pLink)
		size += Point2(EXPAND_SIZEX + EXPAND_GAP, 0);

	return size + Point2((size.y * CHECK_SIZEYPERC) + CHECK_GAP, 0);
}


void MenuItem::onDraw()
{
	float w, h, x, y, elW, elH;
	float checkEnd;

	// draw check
	w = size().x;
	h = size().y;

	elW = h * CHECK_SIZEYPERC;
	elH = h * CHECK_SIZEYPERC;

	checkEnd = elW + CHECK_GAP;

	if (m_bChecked)
	{
		x = 0;
		y = (h - elH) * 0.5f;

		D3DPaint().setFill(RGBA(1,1,1,1), DialogMgr().stdTexture("check"));
		D3DPaint().quad2D(x, y, x + elH, y + elW);
		D3DPaint().draw();
	}

	// draw expander
	if (m_pLink)
	{
		elW = EXPAND_SIZEX;
		elH = h * EXPAND_SIZEYPERC;

		x = w - EXPAND_GAP - EXPAND_SIZEX;
		y = (h - elH) * 0.5f;

		Tri t(Point3(x, y, 1),
				Point3(x + elW, y + elH * 0.5f, 1),
				Point3(x, y + elH, 1));

		D3DPaint().setFill(RGBA(1, 1, 1, 1));
		D3DPaint().tri(t);
		D3DPaint().draw();
	}

	onDraw(checkEnd);
}

// setLink
void MenuItem::setLink(UIMenu* pLink)
{
	m_pLink = pLink;
	setSize(size());
}

// Text Items

MenuItemTextBase::MenuItemTextBase(const std::string &text) : 
	m_text(text)
{
};

MenuItemText::MenuItemText(const std::string &text, const std::string &data, const Delegate& delegate) : 
	MenuItemTextBase(text),
	m_data(data),
	m_delegate(delegate)
{
};

MenuItemText::MenuItemText(const std::string &text, const Delegate& delegate) : 
	MenuItemTextBase(text),
	m_delegate(delegate)
{
};

MenuItemText::MenuItemText(const std::string &text) : 
	MenuItemTextBase(text)
{
};

Point2 MenuItemTextBase::onCalcSize()
{
	setFont(parent()->font());
	return Point2(font().stringWidth(m_text), font().height());
}

void MenuItemTextBase::onDraw(float x)
{
	font().write(D3DPaint(), m_text, x, 0);
	D3DPaint().draw();
}

// Class functions

// addItem
void UIMenu::addItem(const PtrGC<MenuItem>& item)
{
	if (!m_items.empty())
	{
		PtrGC<MenuItem> pBack = m_items.back();
		item->setPos(Point2(BORDER_WIDTH, pBack->pos().y + pBack->size().y));
	}
	else
	{
		item->setPos(Point2(BORDER_WIDTH, itemStart()));
	}

	addChild(item);
	m_items.push_back(item);

	checkWidth(item);
	updateHeight();
	updateItemWidths();
}


// addExpander
PtrGC<UIMenu> UIMenu::addExpander(MenuItem *item)
{
	UIMenu* pLink = addChild(new UIMenu);
	item->setLink(pLink);

	addItem(item);

	pLink->setVisible(false);

	m_subMenus.push_back(pLink);

	pLink->m_pParentMenu = this;

	return pLink;
}


// onDraw
void UIMenu::onDraw()
{
	// draw background
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();

	D3DPaint().setFill(RGBA(0, 0, 0, 0.5f));
	D3DPaint().quad2D(BORDER_WIDTH, itemStart(), size().x - BORDER_WIDTH, size().y - BORDER_HEIGHT);
	D3DPaint().draw();

	// draw header text
	if (!m_title.empty())
	{
		font().write(D3DPaint(), m_title, BORDER_WIDTH, BORDER_HEIGHT);
		D3DPaint().draw();
	}

	// draw selection
	if (m_selected != -1)
	{
		float drawy = itemY(m_selected);
		D3DPaint().setFill(RGBA(1, 0, 0, 0.5f));
		D3DPaint().quad2D(BORDER_WIDTH, drawy, size().x - BORDER_WIDTH, drawy + font().height());
		D3DPaint().draw();
	}
}


// checkWidth
void UIMenu::checkWidth(PtrGC<MenuItem> item)
{
	float width = item->requestedSize().x;
	if (width > size().x)
		setSize(Point2(width + BORDER_WIDTH2 * 2, size().y));
}


// setPos
// overloaded in order to ensure the menu stays within parent bounds
void UIMenu::setPos(const Point2 &pos)
{
	UIElement::setPos(pos);

	Point2 min;
	min.x = BORDER_WIDTH;
	min.y = BORDER_HEIGHT;

	Point2 max;
	max.x = 1 - BORDER_WIDTH;
	max.y = 0.75f - BORDER_HEIGHT;

	ensureInRect(screenPos(), size(), min, max);
}


// onMouseMove
bool UIMenu::onMouseMove(const Point2 &pos, const Point2 &delta)
{
	if (!pointBox(pos, Point2(0, itemStart()), Point2(size().x, size().y - BORDER_HEIGHT)))
	{
		m_selected = -1;
		return true;
	}

	float fontHeight = font().height();
	m_selected = (int)((pos.y - itemStart()) / fontHeight);

	return true;
}


// onMouseOff
void UIMenu::onMouseOff()
{
	m_selected = -1;

	return;
}


// onMouseUp
bool UIMenu::onMouseUp(const Point2 &pos, int button)
{
	if (!isPointIn(clientToScreen(pos)))
	{
		setVisible(false);
		return false;
	}
	else
	{
		if (m_selected == -1 || m_items.size() <= (uint)m_selected)
			return true;

		indexMethod(m_selected);

		PtrGC<MenuItem> item = m_items[m_selected];
		PtrGC<UIMenu> pLink = item->link();

		// normal menu item
		if (!pLink)
		{
			item->onSelected();
			m_selected = -1;
			m_pExpanded = 0;

			for (PtrGC<UIMenu> m = this; m; m = m->m_pParentMenu)
			{
				m->setVisible(false);
			}
		}
		else
		{
			PtrGC<UIMenu> pMenu = pLink;
			expand(*pLink);
		}
	}

	return true;
}

// setVisible
void UIMenu::setVisible(bool bVisible)
{
	UIElement::setVisible(bVisible);

	for (uint i = 0; i < m_subMenus.size(); i++)
	{
		m_subMenus[i]->setVisible(false);
	}

	if (!bVisible && m_bDestroyOnHide)
	{
		if (m_pParent)
			m_pParent->removeChild(this);
	}
}


// itemY
float UIMenu::itemY(uint item)
{
	return itemStart() + font().height() * item;
}


// selectedItem
PtrGC<MenuItem> UIMenu::selectedItem()
{
	if (m_pExpanded)
	{
		return m_pExpanded->selectedItem();
	}

	if (m_selected == -1)
		return 0;

	return m_items[m_selected];
}


// setTitle
void UIMenu::setTitle(const std::string &text)
{
	Point2 newSize(size());

	float width = font().stringWidth(text) + BORDER_WIDTH2;
	if (width > size().x)
		newSize.x = width;

	m_title = text;

	setSize(newSize);

	updateHeight();
}


// itemStart
float UIMenu::itemStart()
{
	if (m_title.empty())
	{
		return BORDER_HEIGHT;
	}
	else
	{
		return font().height() + BORDER_HEIGHT2 + BORDER_HEIGHT;
	}
}


// updateHeight
void UIMenu::updateHeight()
{
	Point2 newSize(size());

	newSize.y = itemStart();

	for (uint i = 0; i < m_items.size(); i++)
	{
		newSize.y += m_items[i]->requestedSize().y;
	}

	newSize.y += BORDER_HEIGHT;

	setSize(newSize);
}


// fillFromDirectory
bool UIMenu::fillFromDirectory(const Path& path, const std::string& wildcard, const MenuItemText::Delegate& method)
{
	WIN32_FIND_DATA data;
	Path fullPath = path.absolute();

	HANDLE h = FindFirstFile((*(fullPath + Path(wildcard))).c_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// don't recurse back or to current directory
			if (std::string(data.cFileName) != "." && std::string(data.cFileName) != "..")
			{
				PtrGC<UIMenu> m = addExpander(new MenuItemText(data.cFileName));
				m->fillFromDirectory(fullPath + Path(data.cFileName), wildcard);
			}
		}
		else
		{
			addItem(new MenuItemText(data.cFileName, *(path + Path(data.cFileName)), method));
		}

		if (!FindNextFile(h, &data))
			break;
	}
	while (1);

	FindClose(h);

	return true;
}

// updateItemWidths
void UIMenu::updateItemWidths()
{
	for (uint i = 0; i < m_items.size(); i++)
	{
		m_items[i]->setSize(Point2(size().x - BORDER_WIDTH * 2, m_items[i]->requestedSize().y));
	}
}

void UIMenu::clearItems()
{
	for (uint i = 0; i < m_items.size(); i++)
		m_items[i].destroy();

	for (uint i = 0; i < m_subMenus.size(); i++)
		m_subMenus[i].destroy();

	m_items.clear();
	m_subMenus.clear();
	setSize(Point2(0, 0));
}

void UIMenu::expand(UIMenu& menu)
{
	// expander
	if (m_pExpanded)
		m_pExpanded->setVisible(false);

	menu.setVisible(true);
	menu.setPos(expandedPosition(menu));

	m_pExpanded = &menu;
}

Point2 UIMenu::expandedPosition(UIMenu& menu)
{
	Point2 expandPos = Point2(size().x, itemY(m_selected));

	// open on left if right pos overlaps
	if (clientToScreen(expandPos).x + menu.size().x > uiBranch()->size().x)
		expandPos.x = -menu.size().x;

	return expandPos;
}
