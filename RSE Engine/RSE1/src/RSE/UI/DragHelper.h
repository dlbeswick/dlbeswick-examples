// ------------------------------------------------------------------------------------------------
//
// DragHelper
// Help with dragging
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UIElement.h"


class DragHelper : public UIElement
{
public:
	DragHelper(bool bAffectSize = false)
	{
		m_tolerance = 0.015f;
		m_bDragging = false;
		m_bAffectSize = bAffectSize;
		setTarget(parent());
	}

	~DragHelper();

	void setTolerance(float f) { m_tolerance = f; }
	void setTarget(const PtrGC<UIElement>& pTarget);
	bool dragging() const { return m_pTarget && m_bDragging; }

	// delegates
	Delegate onDragStart;
	Delegate onDragStop;

protected:
	virtual bool onMouseDown(const Point2& p, int button);
	virtual bool onMouseUp(const Point2& p, int button);
	virtual bool onMousePressed(const Point2& p, int button);

	virtual void start(const Point2& screenMouse);
	virtual void update(const Point2& delta);
	virtual void end();

	virtual void updatePosition();

	bool				m_bAffectSize;
	bool				m_bDragging;
	PtrGC<UIElement>	m_pTarget;
	Point2				m_startDelta;
	Point2				m_lastStartDelta;
	Point2				m_startPosTarget;
	float				m_tolerance;
};


class DialogDragHelper : public DragHelper
{
public:
	DialogDragHelper(bool bAffectSize = false) :
	  DragHelper(bAffectSize)
	{
	}
protected:
	virtual void start(const Point2& screenMouse);
	virtual void updatePosition();
	virtual void end();
};
