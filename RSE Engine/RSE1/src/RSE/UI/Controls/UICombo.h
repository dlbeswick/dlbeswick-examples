// ------------------------------------------------------------------------------------------------
//
// UICombo
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"
#include "RSE/UI/Controls/UIListView.h"

class UIComboBase : public UIElement
{
	USE_RTTI(UIComboBase, UIElement);
public:
	UIComboBase();

	virtual void sizeListSensibly();
	virtual void setSize(const Point2& reqSize);
	virtual bool usesFocus() const { return true; }

	void add(UIElement* l);
	void remove(int idx);
	void select(int idx);
	void clear() { m_listBox->clear(); }

	const PtrGC<UIElement>& selected() const { return m_selected; }
	int selectedIdx() const { return m_listBox->selectedIdx(); }
	UIListViewBase& list() const { return *m_listBox; }

	TPDelegate<const PtrGC<UIElement>&> onSelect;

protected:
	virtual void _onFontChange();

	virtual bool onMouseMove(const Point2 &p, const Point2 &delta) { return false; }
	virtual bool onMouseDown(const Point2 &p, int button);
	virtual bool onMousePressed(const Point2 &p, int button) { return true; }
	virtual bool onMouseUp(const Point2 &p, int button) { return true; }

	virtual bool isMouseOver(const Point2& screenPos);
	virtual void onDraw();
	virtual void update(float delta);

	void onListAdd(const PtrGC<UIElement>&);
	void onListRemove();
	void onListSelect(const PtrGC<UIElement>&);
	void onExpand();

	virtual void adjustListPosAndSize();
	virtual void updateDisplayAreaSize();

	UIListViewBase* m_listBox;
	class UIButton* m_button;
	PtrGC<UIElement> m_selected;
	bool _needsDisplaySizeUpdate;
	bool _needsSensibleListSize;

	Point2 m_displayAreaSize;
	Point2 m_fullSize;
};

template <class ElementType>
class TUICombo : public UIComboBase
{
public:
	TUIListView<ElementType>& list() const { return (TUIListView<ElementType>&)this->list(); }
	PtrGC<ElementType> selected() const { return this->selected().template downcast<ElementType>(); }
};

class UICombo : public TUICombo<ListText>
{
public:
	int selectText(const std::string& s)
	{
		for (uint i = 0; i < m_listBox->numItems(); ++i)
		{
			ListText* t = (ListText*)m_listBox->item(i);
			if (t->text() == s)
			{
				select(i);
				return i;
			}
		}

		return -1;
	}
};
