// ---------------------------------------------------------------------------------------------------------
// 
// UIEditHandle
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "UIEditHandle.h"
#include "Render/D3DPainter.h"
#include "Standard/DXInput.h"


// onDraw
void UIEditHandle::onDraw()
{
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
}


// onMouseMove
bool UIEditHandle::onMouseMove(const Point2& p, const Point2& delta)
{
	if (Input().mousePressed(0))
	{
		if (m_pAffected)
		{
			Point2 min;
			Point2 max;
			Point2 screenP = clientToScreen(p);

			min = m_pAffected->screenPos();
			max = min + m_pAffected->size();

			if (scaling().x < 0)
				min.x = screenP.x;
			else if (scaling().x > 0)
				max.x = screenP.x;

			if (scaling().y < 0)
				min.y = screenP.y;
			else if (scaling().y > 0)
				max.y = screenP.y;

			m_pAffected->setPos(m_pAffected->parent()->screenToClient(min));
			m_pAffected->UIElement::setSize(max - min);

			m_onMove();
		}
	}

	return true;
}


// onMouseDown
bool UIEditHandle::onMouseDown(const Point2& p, int button)
{
	if (button == 0)
	{
		m_startPos = clientToScreen(p);
		m_startASize = m_pAffected->size();
		captureMouse();
	}

	return true;
}


// onMouseUp
bool UIEditHandle::onMouseUp(const Point2& p, int button)
{
	if (button == 0)
	{
		releaseMouse();
	}

	return true;
}

