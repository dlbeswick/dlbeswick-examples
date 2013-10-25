// ------------------------------------------------------------------------------------------------
//
// UIScrollBar
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIScrollBar.h"
#include "Render/D3DPainter.h"
#include "Standard/Collide.h"
#include "Standard/Mapping.h"

const float UIScrollBar::STD_SIZE = 0.025f;
const float MIN_THUMB = 0.0035f;

IMPLEMENT_RTTI(UIScrollBar);

UIScrollBar::UIScrollBar() :
	m_min(0),
	m_max(10),
	m_page(2),
	m_range(10),
	m_bDragging(false),
	m_oldParentSize(0),
	m_thickness(STD_SIZE)
{
	m_pos = 0;
}

void UIScrollBar::onDraw()
{
	// main bar
	D3DPaint().setFill(colour());
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();
	
	// thumb
	D3DPaint().setFill(RGBA(1,1,1));
	D3DPaint().quad2D(m_thumbScreenPos.x - m_thumbScreenSize.x * 0.5f, 
					m_thumbScreenPos.y - m_thumbScreenSize.y * 0.5f, 
					m_thumbScreenPos.x + m_thumbScreenSize.x * 0.5f, 
					m_thumbScreenPos.y + m_thumbScreenSize.y * 0.5f);
	D3DPaint().draw();
}

void UIScrollBar::calcThumbScreen(Point2 barSize)
{
	bool bHorz = isHorz(barSize);
	if (!bHorz)
	{
		std::swap(barSize.x, barSize.y);
	}

	// calc thumb size and pos
	m_thumbScreenSize.y = barSize.y;

	float divisionSize = 0;
	if (m_range)
		divisionSize = std::min(m_page, m_range) / m_range;
	if (divisionSize > 0.9f)
		divisionSize = 0.9f;

	m_thumbScreenSize.x = divisionSize * barSize.x;
	
	if (m_thumbScreenSize.x < MIN_THUMB)
	{
		// thumb size is greater than one element's worth of bar space
		m_thumbScreenSize.x = MIN_THUMB;

		if (!m_page)
			m_thumbScreenPos.x = 0;
		else
		{
			float elementsPerThumb = m_page;

			// calc thumb pos
			m_thumbScreenPos.x = m_thumbScreenSize.x * (Mapping::linear<float>(m_pos, m_min, m_max, 0.0f, 1.0f) / elementsPerThumb);
		}
	}
	else
	{
		// thumb size is equal to one element's worth of bar space

		// calc thumb pos
		m_thumbScreenPos.x = (barSize.x - m_thumbScreenSize.x) * Mapping::linear<float>(m_pos, m_min, m_max, 0.0f, 1.0f);
		m_thumbScreenPos.x += m_thumbScreenSize.x * 0.5f;
	}

	m_thumbScreenPos.y = barSize.y * 0.5f;

	if (!bHorz)
	{
		std::swap(m_thumbScreenPos.x, m_thumbScreenPos.y);
		std::swap(m_thumbScreenSize.x, m_thumbScreenSize.y);
	}
}

void UIScrollBar::setHorz(float length)
{
	length = std::max(length, m_thickness);

	setSize(Point2(length, m_thickness));
}

void UIScrollBar::setVert(float length)
{
	length = std::max(length, m_thickness);

	setSize(Point2(m_thickness, length));
}

void UIScrollBar::setSize(const Point2& reqSize)
{
	calcThumbScreen(reqSize);
	Super::setSize(reqSize);
}

bool UIScrollBar::onMousePressed(const Point2 &p, int button)
{
	m_bDragging = true;
	captureMouse();

	return true;
}

bool UIScrollBar::onMouseMove(const Point2& p, const Point2& delta)
{
	if (!m_bDragging)
		return true;

	Point2 barSize = size();
	Point2 testPos = p;
	Point2 thumbPos = m_thumbScreenPos;
	Point2 thumbSize = m_thumbScreenSize;

	if (!isHorz(size()))
	{
		std::swap(barSize.x, barSize.y);
		std::swap(testPos.x, testPos.y);
		std::swap(thumbPos.x, thumbPos.y);
		std::swap(thumbSize.x, thumbSize.y);
	}	

	if (isHorz(size()))
		thumbPos.x += delta.x;
	else
		thumbPos.x += delta.y;

	float thumbMin = thumbSize.x * 0.5f;
	float thumbMax = barSize.x - thumbSize.x * 0.5f;

	clamp(thumbPos.x, thumbMin, thumbMax);

	// calc pos
	setPos(Mapping::linear(thumbPos.x, thumbMin, thumbMax, m_min, m_max));
	m_onPosChange(); 

	if (!isHorz(size()))
	{
		std::swap(thumbPos.x, thumbPos.y);
	}

	m_thumbScreenPos = thumbPos;

	return true;
}

bool UIScrollBar::onMouseDown(const Point2 &p, int button)
{
	if (!isInThumb(p))
	{
		Point2 testPos = p;
		Point2 thumbPos = m_thumbScreenPos;

		if (!isHorz(size()))
		{
			std::swap(testPos.x, testPos.y);
			std::swap(thumbPos.x, thumbPos.y);
		}	
		
		if (testPos.x < thumbPos.x)
		{
			setPos(m_pos - m_page);
			m_onPosChange(); 
		}
		else
		{
			setPos(m_pos + m_page);
			m_onPosChange(); 
		}

		calcThumbScreen(size());
	}

	return true;
}

bool UIScrollBar::onMouseUp(const Point2 &p, int button)
{
	m_bDragging = false;
	releaseMouse();

	return true;
}

bool UIScrollBar::isInThumb(const Point2& p)
{
	Point2 min(m_thumbScreenPos.x - m_thumbScreenSize.x * 0.5f, 
					m_thumbScreenPos.y - m_thumbScreenSize.y * 0.5f);
	Point2 max(m_thumbScreenPos.x + m_thumbScreenSize.x * 0.5f, 
					m_thumbScreenPos.y + m_thumbScreenSize.y * 0.5f);
	
	return Collide::pointInBox2D(p, min, max);
}

void UIScrollBar::update(float delta)
{
	Super::update(delta);

	if (parent())
	{
		if (isHorz(size()) && m_oldParentSize != parent()->size().x)
		{
			m_oldParentSize = parent()->size().x;
			setHorz(parent()->size().x - m_thickness);
			setPos(Point2(0, parent()->size().y - m_thickness));
			m_onPosChange(); 
		}
		else if (!isHorz(size()) && m_oldParentSize != parent()->size().y)
		{
			m_oldParentSize = parent()->size().y;
			setVert(parent()->size().y - m_thickness);
			setPos(Point2(parent()->size().x - m_thickness, 0));
			m_onPosChange(); 
		}
	}
}
