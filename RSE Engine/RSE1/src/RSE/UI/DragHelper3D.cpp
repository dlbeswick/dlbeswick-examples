// ------------------------------------------------------------------------------------------------
//
// DragHelper3D
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DragHelper3D.h"
#include "Game/Object.h"
#include "Physics/PhysicsObject.h"
#include "UI/DialogMgr.h"
#include <Standard/DXInput.h>

IMPLEMENT_RTTI(DragHelper3D);

void DragHelper3D::updatePosition()
{
	if (!m_pObjTarget)
		return;

	Point3 oldPos(m_pObjTarget->pos());

	Point3 rot(Point3::ZERO);
	float inc = 0.005f;
	if (Input().mousePressed(1))
	{
		if (Input().key(VK_CONTROL))
			rot += Point3((float)Input().mouseDeltaX(), 0, -(float)Input().mouseDeltaY()) * inc;
		else
			m_pObjTarget->setPos(m_pObjTarget->pos() + Point3((float)Input().mouseDeltaX(), 0, -(float)Input().mouseDeltaY()) * inc);
	}
	else
	{
		if (Input().key(VK_CONTROL))
			rot += Point3((float)Input().mouseDeltaX(), -(float)Input().mouseDeltaY(), 0) * inc;
		else
			m_pObjTarget->setPos(m_pObjTarget->pos() + Point3((float)Input().mouseDeltaX(), -(float)Input().mouseDeltaY(), 0) * inc);
	}

	if (rot != Point3::ZERO)
	{
		Quat q(Quat::IDENTITY);
		q.fromZYX(rot);
		m_pObjTarget->setRot(m_pObjTarget->rot() * q);
	}

	// hack, fix
	if (Cast<PhysicsObject>(m_pObjTarget))
	{
		Point3 posDelta = m_pObjTarget->pos() - oldPos;
		m_pObjTarget->setPos(oldPos);
		Cast<PhysicsObject>(m_pObjTarget)->setImpulse(posDelta);
	}
}

void DragHelper3D::start(const Point2& screenMouse)
{
	if (!m_pObjTarget)
		return;

	DialogMgr().pointerLock(true); 
	
	Super::start(screenMouse);
}

void DragHelper3D::end()
{
	DialogMgr().pointerLock(false);
	
	Super::end();
}

bool DragHelper3D::onMousePressed(const Point2& p, int button)
{
	if (button == 0 && captured())
	{
		update(Point2((float)Input().mouseDeltaX(), (float)Input().mouseDeltaY()) * 0.001f);
	}
	
	return m_bDragging;
}

