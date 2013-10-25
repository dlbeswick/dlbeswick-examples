// ------------------------------------------------------------------------------------------------
//
// ObjectQuadtree
// Not adaptive, uses bounding spheres
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Object.h"
#include "Standard/SortedVec.h"


class ObjectQuadtree
{
public:
	ObjectQuadtree(const Point2& min, const Point2& max, const Point2i& divisions = Point2i(0, 0));

	bool add(const PtrGC<Object>& o);
	bool remove(const PtrGC<Object>& o);
	void draw();

	bool isAdded(const PtrGC<Object>& o);

protected:
	bool process(const PtrGC<Object>& o, bool bRemove);

	typedef SortedVec<PtrGC<Object> > Cell;
	typedef std::vector<Cell> Row;
	typedef std::vector<Row> Tree;

	Point2		m_extentMin;
	Point2		m_extentMax;
	Point2		m_extent;
	Point2i		m_divisions;
	Point2		m_divisionSize;
	Tree		m_tree;

	std::set<PtrGC<Object> > m_addedSet;
};