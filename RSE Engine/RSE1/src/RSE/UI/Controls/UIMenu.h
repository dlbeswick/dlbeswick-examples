// ---------------------------------------------------------------------------------------------------------
// 
// UIMenu
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class UIMenu;

// MenuItem
class RSE_API MenuItem : public UIElement
{
public:
	MenuItem() : m_pLink(0), m_bChecked(0) {};

	UIMenu* link() const { return m_pLink; }
	UIMenu& parentMenu() { return *(UIMenu*)m_pParent.ptr(); }
	bool checked() const { return m_bChecked; }
	void setChecked(bool b);
	
	Point2 requestedSize();
	void setLink(UIMenu* pLink);

	virtual void onSelected() = 0;

protected:
	virtual void onDraw(float x) {};
	virtual Point2 onCalcSize() = 0;

	bool m_bChecked;

private:
	virtual void onDraw();

	UIMenu* m_pLink;
	Point2 m_noExpanderSize;
};


// MenuItemText
class RSE_API MenuItemTextBase : public MenuItem
{
public:
	MenuItemTextBase(const std::string& text);
	
	const std::string& text() const { return m_text; }

	virtual void onSelected()
	{
	}

protected:
	virtual Point2 onCalcSize();
	virtual void onDraw(float x);

private:
	std::string m_text;
};

class RSE_API MenuItemText : public MenuItemTextBase
{
public:
	typedef TPDelegate<const MenuItemText&> Delegate;

	MenuItemText(const std::string& text, const std::string &data, const Delegate& delegate);
	MenuItemText(const std::string& text, const Delegate& delegate);
	MenuItemText(const std::string& text);
	
	const std::string& data() const { return m_data; }

	virtual void onSelected()
	{
		m_delegate(*this);
	}

private:
	Delegate	m_delegate;
	std::string m_data;
};


/// Menu item that holds any kind of data.
/// Two constructors are available -- one that accepts a delegate to a function with a parameter of the TMenuItemData type,
/// and one that accepts a delegate with a parameter of the data type itself.
template <class V>
class TMenuItemData : public MenuItemTextBase
{
public:
	typedef TPDelegate<const TMenuItemData<V>&> Delegate;
	typedef TPDelegate<const V&> DelegateData;

	TMenuItemData(const std::string& text, const V& val, const Delegate& delegate = Delegate()) :
		MenuItemTextBase(text),
		_delegate(delegate),
		_val(val)
	{
	}
	
	TMenuItemData(const std::string& text, const V& val, const DelegateData& delegate) :
		MenuItemTextBase(text),
		_delegateData(delegate),
		_val(val)
	{
	}

	const V& data() const { return _val; }
	V& data() { return _val; }

	virtual void onSelected()
	{
		if (_delegateData)
			_delegateData(_val);
		else
			_delegate(*this);
	}

protected:
	V	_val;
	Delegate _delegate;
	DelegateData _delegateData;
};

// Assigns the data in 'val' to 'assign' (by value)
template <class V>
class RSE_API TMenuItemAssign : public MenuItemText
{
public:
	TMenuItemAssign(const std::string& text, V& val, const V& assign, const Delegate& delegate = Delegate()) :
		m_val(val),
		m_assign(assign),
		MenuItemText(text, delegate)
	{
	}
	
	virtual void onSelected()
	{
		m_val = m_assign;
	}

protected:
	virtual void onVisible(bool bVisible)
	{
		setChecked(m_val == m_assign);
	}

	V&	m_val;
	V	m_assign;
};


// TMenuItemDataToggle
// Toggles the data in 'val' between 'assign' and 'nextAssign' (by value)
// Sets val to 'assign'
template <class V>
class RSE_API TMenuItemToggle : public TMenuItemAssign<V>
{
public:
	TMenuItemToggle(const std::string& text, V& val, const V& assign, const V& nextAssign, const typename TMenuItemAssign<V>::Delegate& delegate = typename TMenuItemAssign<V>::Delegate()) :
		m_nextAssign(nextAssign),
		TMenuItemAssign<V>(text, val, assign, delegate)
	{
		val = assign;
	}
	
	virtual void onSelected()
	{
		if (this->m_val == this->m_assign)
			this->m_val = m_nextAssign;
		else
			this->m_val = this->m_assign;
	}

protected:
	V	m_nextAssign;
};

// UIMenu
class RSE_API UIMenu : public UIElement
{
public:
	UIMenu() : m_selected(-1), m_pExpanded(0), m_pParentMenu(0)
	{
		setDestroyOnHide(false);
		setInheritClipping(false);
	};

	~UIMenu()
	{
	};

	TPDelegate<int> indexMethod;

	void setTitle(const std::string &text);

	// add functions
	void addItem(const PtrGC<MenuItem>& pItem);
	PtrGC<UIMenu> addExpander(MenuItem* pItem);
	bool fillFromDirectory(const Path& path, const std::string& wildcard = "*.*", const MenuItemText::Delegate& method = MenuItemText::Delegate());

	void clear()							{ clearItems(); }
	void clearItems();
	void updateItemWidths();

	// info
	PtrGC<MenuItem> selectedItem();
	const int selected() const		{ return m_selected; }
	MenuItem& item(int i)			{ return *m_items[i]; }
	void setDestroyOnHide(bool b)	{ m_bDestroyOnHide = b; }

	// overloads
	virtual void setPos(const Point2 &pos);
	virtual void setVisible(bool bVisible);
	
protected:
	virtual void onDraw();

	virtual bool isMouseOver(const Point2&) { return true; }

	// input
	virtual bool onMouseDown(const Point2 &, int) { return true; }
	virtual bool onMouseUp(const Point2 &pos, int button);
	virtual bool onMouseMove(const Point2 &pos, const Point2 &delta);
	virtual bool onMousePressed(const Point2 &, int) { return true; }
	virtual void onMouseOff();

	virtual void checkWidth(PtrGC<MenuItem> pItem);
	virtual float itemY(uint item);
	virtual float itemStart();
	virtual void updateHeight();
	virtual void expand(UIMenu& menu);
	virtual Point2 expandedPosition(UIMenu& menu);

	bool m_bDestroyOnHide;
	std::vector<PtrGC<MenuItem> > m_items;
	std::vector<PtrGC<UIMenu> > m_subMenus;
	std::string m_title;
	int m_selected;
	PtrGC<UIMenu> m_pExpanded;
	PtrGC<UIMenu> m_pParentMenu;
};

// UIMenuText
class RSE_API UIMenuText : public UIMenu
{
public:
	PtrGC<MenuItemText> selectedItem() { return UIMenu::selectedItem().downcast<MenuItemText>(); }

	PtrGC<MenuItemText> itemByText(const std::string& text)
	{
		for (uint i = 0; i < m_items.size(); i++)
			if ((m_items[i].downcast<MenuItemText>())->text() == text)
				return m_items[i].downcast<MenuItemText>();

		return 0;
	}
};
