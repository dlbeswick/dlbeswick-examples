// ------------------------------------------------------------------------------------------------
//
// PhysicsMgr
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_PHYSICSMGR_H_
#define RSE_PHYSICSMGR_H_

#include "RSE/Physics/PhysicsObject.h"
#include "Standard/PendingList.h"

class Level;

class RSE_API PhysicsMgr
{
public:
	typedef PendingListNullRemover<PtrGC<PhysicsObject> > Objects;
	typedef CollisionInfo HitRecord;

	PhysicsMgr(Level& l);
	~PhysicsMgr();

	void clear();
	void remove(const PtrGC<PhysicsObject>& o);

	void setUseCollisions(bool b);

	void update(float delta);
	void drawDebug();

	typedef std::vector<const Tri*> TriRefList;

	EmbeddedPtr<CollisionGroup> addCollisionGroup();

	// collision funcs
	class CollisionFunc
	{
	public:
		virtual bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const = 0;
	};

	HitRecord findFirstCollision(const PtrGC<PhysicsObject>& o0);
	CollisionInfo::HitResult detectCollision(const PtrGC<PhysicsObject>& o0, const PtrGC<PhysicsObject>& o1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1);

private:
	class HitResolution
	{
	public:
		HitResolution() {}

		HitResolution(const PtrGC<PhysicsObject>& _subject, const Point3& _impulse) :
			impulse(_impulse), subject(_subject)
		{
		}

		Point3 impulse;
		PtrGC<PhysicsObject> subject;
	};

	friend class PhysicsObject;

	typedef std::vector<HitRecord> HitRecords;
	typedef std::vector<HitResolution> HitResolutions;

	void step(float delta);

	void addCollisionHandler(const RTTI* lhs, const RTTI* rhs, CollisionFunc* func);

	void clearVis();

	void findCollisions(CollisionGroup& g, const PtrGC<PhysicsObject>& o0, float delta, int step, HitRecords& hits);
	void collisionResponse(CollisionInfo& info, HitResolutions& resolutions, float delta, int step);
	void applyFriction(Point3& vel, const Point3& normal, const Point3& friction, float coefficient, float delta);
	void propagateMomentum(CollisionInfo& info, float momentum, HitResolutions& resolutions);
	void clampVelocity(PhysicsObject& o);

	Level&							m_level;

	float							m_accumulator;
	int								m_updateFreq;
	int								m_maxSteps;
	int								m_maxDynamicSteps;
	bool							m_useCollisions;

	typedef std::vector<EmbeddedPtr<CollisionGroup> > CollisionGroups;
	CollisionGroups					m_groups;
	Objects							m_objects;

	std::vector<CollisionInfo>		m_visHit;
	std::vector<const Tri*>			m_visHitPoly;

	typedef stdext::hash_map<const RTTI*, stdext::hash_map<const RTTI*, CollisionFunc*> > CollisionMap;
	CollisionMap m_collisionMap;
};

#endif
