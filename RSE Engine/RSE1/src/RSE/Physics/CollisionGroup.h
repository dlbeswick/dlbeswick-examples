#ifndef RSE_COLLISIONGROUP_H_
#define RSE_COLLISIONGROUP_H_

class Level;
class PhysicsObject;

#include "Standard/EmbeddedPtr.h"
#include "Standard/PtrGC.h"
#include "Standard/PendingList.h"

// collision groups
class RSE_API CollisionGroup : public EmbeddedPtrHost
{
public:
	typedef PendingListNullRemover<PtrGC<PhysicsObject> > Objects;

	CollisionGroup(Level& level) :
		m_level(level)
	{
	};

	~CollisionGroup();

	void add(const PtrGC<PhysicsObject>& o);
	bool contains(const PtrGC<PhysicsObject>& o);
	void remove(const PtrGC<PhysicsObject>& o);

protected:
	Objects	m_objects;
	Level& m_level;

	friend class PhysicsMgr;
};

#endif
