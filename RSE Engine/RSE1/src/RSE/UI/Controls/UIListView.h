// ------------------------------------------------------------------------------------------------
//
// UIListView
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class ListItem;

class RSE_API UIListViewBase : public UIElement
{
	USE_RTTI(UIListViewBase, UIElement);
public:
	UIListViewBase();

	void add(UIElement* item);
	void remove(int idx);
	void clear() { while (numItems()) remove(0); }
	bool empty() { return m_items.size() == 0; }
	void sort();
	void select(int idx);
	void setRows(int i) { m_rows = i; m_columns = 0; m_bRecalcLayout = true; }
	void setColumns(int i) { m_columns = i; m_rows = 0; m_bRecalcLayout = true; }
	
	UIElement* item(int idx);
	UIElement* selected() const { if (m_selected >= 0) return m_items[m_selected]; return 0; }
	int selectedIdx() const { return m_selected; }
	uint numItems() { return m_items.size(); }

	// returns the total size of all items added to the listbox
	Point2 sizeOfAllItems();

	// delegates
	TPDelegate<const PtrGC<UIElement>&> onAdd;
	Delegate onRemove;
	TPDelegate<const PtrGC<UIElement>&> onSelect;
	TPDelegate<const PtrGC<UIElement>&> onDoubleClick;

	bool mouseMove(const Point2 &p, const Point2 &delta) { return true; }
	bool mouseDown(const Point2 &p, int button);
	bool mousePressed(const Point2 &p, int button) { return true; }
	bool mouseDouble(const Point2 &p, int button);

protected:

	virtual void onDraw();

	void calcLayout();
	Point2 calcItemSize(Point2& largest);

	class ScrollingContainer* _container;

	typedef std::vector<UIElement*> Items;
	Items	m_items;

	int		m_rows;
	int		m_columns;
	bool	m_bRecalcLayout;
	Point2	m_cellSize;
	Point2	m_allItemsSize;
	int		m_realCols;
	int		m_realRows;

	int		m_selected;
};

template <class ElementType>
class TUIListView : public UIListViewBase
{
public:
	ElementType* item(int idx) { return (ElementType*)UIListViewBase::item(idx); }
	ElementType* selected() const { return (ElementType*)UIListViewBase::selected(); }
};

class RSE_API ListItem : public UIElement
{
public:
	UIListViewBase* list() { return (UIListViewBase*)parent().ptr(); }
};

class RSE_API ListText : public ListItem
{
public:
	ListText(const std::string &text, const std::string &data = std::string());

	void setText(const std::string& text) { m_text = text; _onFontChange(); }
	void setData(const std::string& data) { m_data = data; }

	std::string& text() { return m_text; }
	std::string& data() { return m_data; }

protected:
	virtual void _onFontChange();
	virtual void onDraw();

	std::string m_text;
	std::string m_data;
};

class RSE_API UIListView : public TUIListView<ListText>
{
	USE_RTTI(UIListView, TUIListView<ListText>);
};