// ------------------------------------------------------------------------------------------------
//
// UIScrollBar
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UI/UIElement.h"


class UIScrollBar : public UIElement
{
	USE_RTTI(UIScrollBar, UIElement);
public:
	UIScrollBar();

	static const float STD_SIZE;

	virtual void setSize(const Point2& reqSize);

	void setThickness(float f)				{ m_thickness = f; }
	void setExtent(float min, float max)	{ m_min = min; m_max = max; m_range = max - min; m_page = m_range / 10.0f; calcThumbScreen(size()); }
	void setPage(float page)				{ m_page = page; calcThumbScreen(size()); }
	void setPos(float pos)					{ doSetPos(pos); }
	void setPos(const Point2& pos)			{ UIElement::setPos(pos); }
	float scrollPos()						{ return m_pos; }

	void setHorz(float length);
	void setVert(float length);
	float min() { return m_min; }
	float max() { return m_max; }
	float range() { return m_max - m_min; }

	void setOnPosChange(const Delegate& m)	{ m_onPosChange = m; }

protected:
	virtual void update(float delta);
	virtual void onDraw();
	
	virtual bool onMouseDown(const Point2 &p, int button);
	virtual bool onMousePressed(const Point2 &p, int button);
	virtual bool onMouseUp(const Point2 &p, int button);
	virtual bool onMouseMove(const Point2 &p, const Point2 &delta);

	void	calcThumbScreen(Point2 barSize);
	bool	isHorz(const Point2& s)			{ return s.x > s.y; }
	bool	isInThumb(const Point2& p);

	void	doSetPos(float newPos)			{ m_pos = newPos; clamp(m_pos, m_min, m_max); calcThumbScreen(size()); }

	float	m_pos;
	float	m_min;
	float	m_max;
	float	m_page;
	float	m_range;
	float	m_thickness;
	float	m_oldParentSize;
	Point2	m_thumbScreenPos;
	Point2	m_thumbScreenSize;

	bool	m_bDragging;

	Delegate m_onPosChange;
};