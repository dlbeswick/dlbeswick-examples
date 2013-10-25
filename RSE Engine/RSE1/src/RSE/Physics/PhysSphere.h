// ------------------------------------------------------------------------------------------------
//
// PhysSphere
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "PhysicsObject.h"


class RSE_API PhysSphere : public PhysicsObject
{
	USE_RTTI(PhysSphere, PhysicsObject);
public:
	PhysSphere(float radius = 1) :
	  m_radius(radius)
	{
	}

	PhysSphere(const Point3& extent) :
		m_radius(std::max(extent.x, std::max(extent.y, extent.z)))
	{
	}

	float radius() const { return m_radius; }
	virtual float sphereBound() const { return m_radius; }
	void setRadius(float radius) { m_radius = radius; } // use with care, could cause penetration
	virtual void draw();

	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list);
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u);

private:
	float m_radius;
};