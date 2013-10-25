// ------------------------------------------------------------------------------------------------
//
// ObjectQuadtree
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "ObjectQuadtree.h"
#include "Render/D3DPainter.h"
#include <Standard/Collide.h>

ObjectQuadtree::ObjectQuadtree(const Point2& min, const Point2& max, const Point2i& divisions) :
	m_extentMin(min), m_extentMax(max), m_divisions(divisions)
{
	assert(m_extentMin.x < m_extentMax.x);
	assert(m_extentMin.y < m_extentMax.y);

	m_extent = Point2(m_extentMax - m_extentMin);

	if (m_divisions.x == 0)
		m_divisions = Point2i(20, 20);

	m_divisionSize(m_extent.x / m_divisions.x, m_extent.y / m_divisions.y);

	// setup array
	m_tree.resize(m_divisions.y);
	for (int i = 0; i < m_divisions.y; i++)
	{
		m_tree[i].resize(m_divisions.x);
	}
}

bool ObjectQuadtree::add(const PtrGC<Object>& o)
{
	return process(o, false);
}

bool ObjectQuadtree::remove(const PtrGC<Object>& o)
{
	return process(o, true);
}

// balance of speed and code reuse
bool ObjectQuadtree::process(const PtrGC<Object>& o, bool bRemove)
{
	// calculate potential coverage via a rectangle
	Point2 extent(o->extent().x * 0.5f, o->extent().y * 0.5f);
	Point2 objPoint(o->pos().x + o->geomCenter().x, o->pos().y + o->geomCenter().y);
	Point2 objMin(objPoint - extent);
	Point2 objMax(objPoint + extent);

	Point2i start;
	Point2i end;
	start.x = (int)floor((objMin.x - m_extentMin.x) / m_divisionSize.x);
	start.y = (int)floor((objMin.y - m_extentMin.y) / m_divisionSize.y);
	end.x = (int)ceil((objMax.x - m_extentMin.x) / m_divisionSize.x);
	end.y = (int)ceil((objMax.y - m_extentMin.y) / m_divisionSize.y);

	clamp(start.x, 0, m_divisions.x);
	clamp(start.y, 0, m_divisions.y);
	clamp(end.x, 0, m_divisions.x);
	clamp(end.y, 0, m_divisions.y);

	if (!bRemove)
	{
		m_addedSet.insert(o);

		for (int y = start.y; y < end.y; ++y)
		{
			for (int x = start.x; x < end.x; ++x)
			{
				assert(m_tree[y][x].find(o) == m_tree[y][x].end());
				m_tree[y][x].insert(o);
			}
		}
	}
	else
	{
		m_addedSet.erase(o);

		for (int y = start.y; y <= end.y; ++y)
		{
			for (int x = start.x; x <= end.x; ++x)
			{
				assert(m_tree[y][x].find(o) != m_tree[y][x].end());
				m_tree[y][x].remove(o);
			}
		}
	}

	return true;
}

void ObjectQuadtree::draw()
{
	D3DPaint().reset();
	D3DPaint().setFill(RGBA(0, 1, 0));

	for (int i = 0; i <= m_divisions.y; i++)
	{
		float y = m_extentMin.y + m_divisionSize.y * i;
		D3DPaint().line(Point3(m_extentMin.x, y, 0), Point3(m_extentMax.x, y, 0));
	}

	for (int i = 0; i <= m_divisions.x; i++)
	{
		float x = m_extentMin.x + m_divisionSize.x * i;
		D3DPaint().line(Point3(x, m_extentMin.y, 0), Point3(x, m_extentMax.y, 0));
	}

	// draw occupied cells
	D3DPaint().setFill(RGBA(1, 0, 0));

	for (int i = 0; i < m_divisions.y; i++)
	{
		for (int j = 0; j < m_divisions.x; j++)
		{
			if (!m_tree[i][j].empty())
			{
				Point3 min = Point3(m_divisionSize.x * j, m_divisionSize.y * i, 0);
				Point3 max = Point3(m_divisionSize.x * (j + 1), m_divisionSize.y * (i + 1), 0);

				min.x += m_extentMin.x;
				min.y += m_extentMin.y;
				max.x += m_extentMin.x;
				max.y += m_extentMin.y;

				D3DPaint().quad(
					Quad(
						Point3(min.x, min.y, 0),
						Point3(max.x, min.y, 0),
						Point3(max.x, max.y, 0),
						Point3(min.x, max.y, 0)
						)
				);
			}
		}
	}
}

bool ObjectQuadtree::isAdded(const PtrGC<Object>& o)
{
	return m_addedSet.find(o) != m_addedSet.end();
}