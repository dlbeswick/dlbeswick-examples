// ---------------------------------------------------------------------------------------------------------
// 
// UIEditHandle
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "UI/UIElement.h"


class RSE_API UIEditHandle : public UIElement
{
public:
	UIEditHandle(const Delegate& onMove) :
	  m_onMove(onMove)
	{
		setSize(Point2(0.01f, 0.01f));
		setColour(RGBA(1, 1, 1, 1));
	}

	const Point2& scaling() { return m_scaling; }
	void setScaling(const Point2& s) { m_scaling = s; }

	void setAffected(const PtrGC<UIElement>& a) { m_pAffected = a; }

protected:
	// draw
	virtual void onDraw();

	// input
	virtual bool onMouseDown(const Point2& p, int button);
	virtual bool onMousePressed(const Point2& p, int button) { return true; }
	virtual bool onMouseUp(const Point2& p, int button);

	virtual bool onMouseMove(const Point2& p, const Point2& delta);

	Point2 m_scaling;
	PtrGC<UIElement> m_pAffected;
	Point2 m_startPos;
	Point2 m_startASize;
	Delegate m_onMove;
};