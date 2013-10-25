// ------------------------------------------------------------------------------------------------
//
// DragHelper
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DragHelper.h"
#include "Render/OSWindow.h"
#include "Render/SDeviceD3D.h"
#include "UI/DialogMgr.h"
#include <Standard/DXInput.h>

DragHelper::~DragHelper()
{
	if (dragging())
		end();
}

void DragHelper::setTarget(const PtrGC<UIElement>& pTarget)
{
	m_pTarget = pTarget;
	if (pTarget)
		setSize(pTarget->size());
}

bool DragHelper::onMouseDown(const Point2& p, int button)
{
	if (button == 0)
		start(clientToScreen(p));

	return m_bDragging;
}

bool DragHelper::onMouseUp(const Point2& p, int button)
{
	bool wasDragging = m_bDragging;

	if (button == 0)
		end();

	return wasDragging;
}

bool DragHelper::onMousePressed(const Point2& p, int button)
{
	if (button == 0 && captured())
	{
		update(DialogMgr().pointerDelta());
	}
	
	return m_bDragging;
}

void DragHelper::start(const Point2& screenMouse)
{
	if (!m_pTarget)
		return;

	m_startDelta = Point2::ZERO;
	if (!m_bAffectSize)
		m_startPosTarget = m_pTarget->screenPos();
	else
		m_startPosTarget = m_pTarget->size();
	captureMouse();

	if (m_tolerance == 0)
		m_bDragging = true;

	onDragStart();
}

void DragHelper::update(const Point2& delta)
{
	if (!m_pTarget)
		return;

	m_startDelta += delta;

	if (m_bDragging || m_startDelta.length() >= m_tolerance)
	{
		m_bDragging = true;
		if (m_lastStartDelta != m_startDelta)
		{
			updatePosition();
		}
	}

	m_lastStartDelta = m_startDelta;
}

void DragHelper::updatePosition()
{
	if (!m_bAffectSize)
		m_pTarget->setPos(m_pTarget->screenToParent(m_startPosTarget + m_startDelta));
	else
		m_pTarget->setSize(m_startPosTarget + m_startDelta);
}

void DragHelper::end()
{
	releaseMouse();
	m_bDragging = false;
	onDragStop();
}

void DialogDragHelper::start(const Point2& screenMouse)
{
	DragHelper::start(screenMouse);

	Dialog* dlgParent = Cast<Dialog>(m_pTarget.ptr());
	if (dlgParent && dlgParent->osManaged())
	{
		if (!m_bAffectSize)
			SendMessage(dlgParent->osWindow()->hWnd(), WM_SYSCOMMAND, SC_MOVE + 2, 0);
		else
			SendMessage(dlgParent->osWindow()->hWnd(), WM_SYSCOMMAND, SC_SIZE + 8, 0);
		end();
	}
}

void DialogDragHelper::updatePosition()
{
	Dialog* dlgParent = Cast<Dialog>(m_pTarget.ptr());
	if (dlgParent && !dlgParent->osManaged())
		DragHelper::updatePosition();
}

void DialogDragHelper::end()
{
	DragHelper::end();
}