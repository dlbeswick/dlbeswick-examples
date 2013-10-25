// ------------------------------------------------------------------------------------------------
//
// PhysPlane
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE1_PHYSPLANE_H
#define RSE1_PHYSPLANE_H

#include "PhysicsObject.h"
#include <Standard/Dirty.h>

class RSE_API PhysPlane : public PhysicsObject
{
	USE_RTTI(PhysPlane, PhysicsObject);
public:
	PhysPlane() : m_normal(0, 0, 1) {}

	PhysPlane(const Point3& _normal, const Point3& pos)
	{
		setPos(pos);
		m_normal = _normal;
	}

	const Point3& normal() const;
	const Point3& localNormal() const { return m_normal; }

	virtual void setRot(const Quat& q)
	{
		if (q != rot())
			m_worldNormal.setDirty();

		Super::setRot(q);
	}

	virtual void draw();

	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list) {};

	// return true with u if collision occurred
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u) { return false; }

	virtual DMath::Plane plane() const;

private:
	Point3 m_normal;
	mutable Dirty<Point3> m_worldNormal;
};

#endif