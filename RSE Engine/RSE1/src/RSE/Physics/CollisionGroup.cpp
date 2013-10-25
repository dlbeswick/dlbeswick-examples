#include "RSE/pch.h"
#include "RSE/Physics/CollisionGroup.h"
#include "RSE/Physics/PhysicsObject.h"

// Collision groups
CollisionGroup::~CollisionGroup()
{
	for (Objects::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
		(*i)->m_addRef = 0;
}

void CollisionGroup::add(const PtrGC<PhysicsObject>& o)
{
	assert(!m_objects.contains(o));
	m_objects.add(o);
	assert(&o->level() == &m_level);
	o->m_groups.insert(this);
	o->incAddRef();
}

bool CollisionGroup::contains(const PtrGC<PhysicsObject>& o)
{
	return m_objects.contains(o);
}

void CollisionGroup::remove(const PtrGC<PhysicsObject>& o)
{
	if (m_objects.remove(o))
		o->decAddRef();
	o->m_groups.erase(this);
}

