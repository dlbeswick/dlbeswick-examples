// ------------------------------------------------------------------------------------------------
//
// PhysCylinderAA
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "PhysicsObject.h"


class RSE_API PhysCylinderAA : public PhysicsObject
{
	USE_RTTI(PhysCylinderAA, PhysicsObject);
public:
	PhysCylinderAA(float radius = 1, float height = 1) :
	  m_radius(radius), m_height(height)
	{
	}

	PhysCylinderAA(const Point3& extent) :
		m_radius(std::max(extent.x, extent.y) * 0.5f), 
		m_height(extent.z)
	{
	}

	float radius() { return m_radius; }
	float height() { return m_height; }
	virtual void draw();

	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list) {};
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u) { return false; }

private:
	float m_radius;
	float m_height;
};