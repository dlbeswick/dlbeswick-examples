// ------------------------------------------------------------------------------------------------
//
// UILayout
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class RSE_API UILayout : public UIContainer
{
	USE_RTTI(UILayout, UIContainer);
public:
	UILayout();

	void forceRecalc();

	virtual void update(float delta);

	virtual void setSize(const Point2& s);

	virtual void setParent(const PtrGC<UIElement>& pParent);
	virtual UIElement* _addChild(const PtrGC<UIElement>& child);
	virtual void removeChild(const PtrGC<UIElement>& child);

	bool bindToParent;

protected:
	virtual UIElement* addChildExplicit(const PtrGC<UIElement>& child);
	virtual void recalc() = 0;
	virtual void onDraw();

	ElementList m_layoutChildren;
	bool m_needsRecalc;

	void onParentSizeChanged();
	virtual void onFontChange();
};

class RSE_API UILayoutFill : public UILayout
{
	USE_RTTI(UILayoutFill, UILayout);
	
protected:
	virtual void recalc();
};

class RSE_API UILayoutGrid : public UILayout
{
	USE_RTTI(UILayoutGrid, UILayout);
public:
	UILayoutGrid() :
	  m_cols(1),
	  m_bFillMethod(true)
	{
	}

	void setCols(int cols) { m_cols = cols; }
	void setFillMethod(bool bLeftToRight) { m_bFillMethod = bLeftToRight; }

protected:
	virtual void recalc();
	
	int m_cols;
	bool m_bFillMethod;
};

class RSE_API UILayoutTable : public UILayout
{
	USE_RTTI(UILayoutTable, UILayout);
public:
	UILayoutTable();

	ALIGN cellAlign;
	VALIGN cellVAlign;

	void setCols(int cols, bool autoSize = true);
	void setCol(int idx, float width);	// expressed in fraction of width
	void setRowSpacing(float y);

protected:
	virtual void recalc();
	
	typedef std::vector<float> Cols;
	Cols m_cols;
	
	float m_rowSpacing;

	typedef std::vector<UIElement*> LayoutElements;
	LayoutElements m_layoutElements;
};