// ------------------------------------------------------------------------------------------------
//
// PhysicsObject
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "PhysicsObject.h"
#include "AppBase.h"
#include "PhysicsMgr.h"
#include "Game/Level.h"

IMPLEMENT_RTTI(PhysicsObject);

// NOTE maybe make inline
bool CollisionInfo::operator < (const CollisionInfo& rhs) const
{
	return u < rhs.u;
}

CollisionInfo CollisionInfo::mirrored() const
{
	CollisionInfo result = *this;
	
	std::swap(result.collider, result.victim);
	result.normal = -result.normal;

	return result;
}

PtrGC<class PhysicsObject> CollisionInfo::other(const PtrGC<class PhysicsObject>& reference) const
{
	if (victim == reference)
	{
		return collider;
	}
	else
	{
		return victim;
	}
}

////

PhysicsObject::PhysicsObject() :
	m_vel(Point3::ZERO),
	m_angularVel(Point3::ZERO),
	m_impulse(Point3::ZERO),
	m_mass(1),
	m_friction(0),
	m_gravityScale(1),
	m_maxVel(Point3::MAX),
	m_elasticity(0),
	m_addRef(0)
{
	zero(m_material);
	setColour(RGBA(1,1,1));
}

PhysicsObject::~PhysicsObject()
{
}

bool PhysicsObject::hasHitNormal(const Point3& normal, float tolerance)
{
	for (uint i = 0; i < m_lastHits.size(); ++i)
	{
		float dot = m_lastHits[i].normal * normal;
		if (dot > 0 && dot >= 1.0f - tolerance)
			return true;
	}
	
	return false;
}

void PhysicsObject::incAddRef()
{
	if (!m_addRef)
		level().physicsMgr().m_objects.add(this);

	++m_addRef;
}

void PhysicsObject::decAddRef()
{
	--m_addRef;
	assert(m_addRef >= 0);

	if (m_addRef == 0)
		level().physicsMgr().m_objects.remove(this);
}

void PhysicsObject::levelChanging(Level* l)
{
	Super::levelChanging(l);
	removeFromGroups();

	if (hasLevel())
		level().physicsMgr().m_objects.remove(this);
}

void PhysicsObject::applyForce(const Point3& v)
{
	applyImpulse(v / mass());
}

bool PhysicsObject::visible() const
{
	return Super::visible() && AppBase().options().get<bool>("Debug","ShowPhysicsDebug");
}

void PhysicsObject::removeFromGroups()
{
	while (!m_groups.empty())
	{
		(*m_groups.begin())->remove(this);
	}
}
