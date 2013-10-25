// ------------------------------------------------------------------------------------------------
//
// PhysRay
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "PhysicsObject.h"


class RSE_API PhysRay : public PhysicsObject
{
	USE_RTTI(PhysRay, PhysicsObject);
public:
	PhysRay() {}

	PhysRay(const Point3& dir, const Point3& origin = Point3::ZERO) :
	  m_dir(dir)
	{
		setPos(origin);
	}

	const Point3& dir() const { return m_dir; }
	virtual void draw();

	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list) {};
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u) { return false; }

private:
	Point3 m_dir;
};