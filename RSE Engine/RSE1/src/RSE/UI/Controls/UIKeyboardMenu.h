// ------------------------------------------------------------------------------------------------
//
// UIKeyboardMenu
// Adding this control to a dialog enables old-school style keyboard selection.
// Up/down moves selection along tab-order. The selected element's children are walked until
// a control capable of accepting keyboard focus is found, then keyboard events are delegated to
// it. So, to have the selection arrows encompass a group of controls (i.e. label and checkbox) then
// group them within a UIElement control.
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class RSE_API UIKeyboardMenuBase : public UIElement
{
	USE_RTTI(UIKeyboardMenuBase, UIElement);
public:
	UIKeyboardMenuBase();

	int prevKey;
	int nextKey;

	virtual void add(const PtrGC<UIElement>& e);

	virtual void update(float delta);
	virtual bool usesFocus() const { return true; }

	virtual void keyDown(int key);
	virtual void keyPressed();
	virtual void keyUp(int key);
	virtual void keyChar(int key);

protected:
	virtual void onSelected() = 0;

	virtual PtrGC<UIElement> prev(const PtrGC<UIElement>& p);
	virtual PtrGC<UIElement> next(const PtrGC<UIElement>& p);
	virtual void select(const PtrGC<UIElement>& p);
	virtual PtrGC<UIElement> keyboardFocusFor(const PtrGC<UIElement>& p);

	virtual bool usesKey(int key);

	PtrGC<UIElement> m_keyboardFocus;
	PtrGC<UIElement> m_selection;
	ElementList m_items;
};

class RSE_API UIKeyboardMenu : public UIKeyboardMenuBase
{
	USE_RTTI(UIKeyboardMenu, UIKeyboardMenuBase);
public:
	UIKeyboardMenu();

	void setArrowSize(const Point2& sizeLeft, const Point2& sizeRight);

	// style
	UISTYLE
		std::string leftArrowMaterialName;
		std::string rightArrowMaterialName;
		Point2 leftArrowSize;
		Point2 rightArrowSize;
	UISTYLEEND

protected:
	virtual void construct();

	UIElement* m_arrows[2];

	virtual void onSelected();
};