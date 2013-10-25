// ------------------------------------------------------------------------------------------------
//
// DragHelper3D
// Help with dragging
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "DragHelper.h"


class DragHelper3D : public DragHelper
{
	USE_RTTI(DragHelper3D, DragHelper);
public:
	DragHelper3D()
	{
		if (parent())
			setSize(parent()->size());
		else
			setSize(Point2(1,1));
	}

	void setTarget(const PtrGC<class Object>& pTarget) { m_pObjTarget = pTarget; }

	bool dragging() const { return m_pObjTarget && m_bDragging; }


protected:
	virtual void start(const Point2& screenMouse);
	virtual void end();

	virtual void updatePosition();

	virtual bool onMousePressed(const Point2& p, int button);

	PtrGC<class Object>		m_pObjTarget;
};