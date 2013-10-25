// ------------------------------------------------------------------------------------------------
//
// PhysicsObject
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_PHYSICSOBJECT_H_
#define RSE_PHYSICSOBJECT_H_

#include "RSE/Game/Object.h"
#include "RSE/Physics/CollisionGroup.h"

class RSE_API CollisionInfo
{
public:
	enum HitResult
	{
		NOHIT,
		HIT,
		CONTACT
	};

	CollisionInfo()
	{
		processHit = true;
		momentumPropagated = false;
		friction = Point3::ZERO;
	}

	CollisionInfo(const Point3& _normal, float _u, const PtrGC<class PhysicsObject>& _collider, const PtrGC<class PhysicsObject>& _victim, HitResult _result)
	{
		victim = _victim;
		normal = _normal;
		u = _u;
		collider = _collider;
		processHit = true;
		momentumPropagated = false;
		result = _result;
		friction = Point3::ZERO;
	}

	PtrGC<class PhysicsObject> other(const PtrGC<class PhysicsObject>& reference) const;

	Point3 point;
	Point3 normal;
	Point3 friction;
	float u;
	PtrGC<class PhysicsObject> collider;
	PtrGC<class PhysicsObject> victim;
	bool processHit;
	bool momentumPropagated;
	HitResult result;

	bool operator < (const CollisionInfo& rhs) const;

	CollisionInfo mirrored() const;
};

class RSE_API PhysicsObject : public Object
{
	USE_RTTI(PhysicsObject, Object);
public:
	PhysicsObject();
	virtual ~PhysicsObject();

	const Point3&	vel() const				{ return m_vel; }
	const Point3&	angularVel() const		{ return m_angularVel; }
	const Point3&	maxVel() const			{ return m_maxVel; }
	float			mass() const			{ return m_mass; }
	float			friction() const		{ return m_friction; }

	// fix -- "elasticity" is operating contrary to the proper definition.
	// current when m_elasticity is 0, collisions are perfectly elastic. when 1, they're inelastic. 
	// it should be the inverse.
	float			elasticity() const		{ return m_elasticity; }

	float			gravityScale() const	{ return m_gravityScale; }
	const Point3&	impulse() const			{ return m_impulse; }
	const PtrGC<Object> associatedObj() const { return m_associatedObj; }
	bool			immovable() const		{ return m_mass == FLT_MAX; }
	virtual float	sphereBound() const		{ return 0; }
	virtual Point3	extent() const			{ float bound = sphereBound() * 2.0f; return Point3(bound,bound,bound); }
	virtual void removeFromGroups();
	virtual bool visible() const;

	virtual void applyForce(const Point3& v);

	void setImpulse(const Point3& v)	{ m_impulse = v; } // impulse: change in velocity applied over one frame only
	void applyImpulse(const Point3& v)	{ m_impulse += v; } // impulse: change in velocity applied over one frame only
	void setVel(const Point3& v)		{ m_vel = v; }
	void setAngularVel(const Point3& v)	{ m_angularVel = v; }
	void setMaxVel(const Point3& v)		{ m_maxVel = v; }
	void setMass(float m)				{ m_mass = m; }
	void setFriction(float coefficient)	{ m_friction = coefficient; }
	void setElasticity(float f)			{ m_elasticity = f; }
	void setGravityScale(float f)		{ m_gravityScale = f; }
	void setColour(const RGBA& col)		{ m_material.Diffuse = col; }
	void setAssociatedObj(const PtrGC<Object>& obj) { m_associatedObj = obj; }

	typedef std::vector<const Tri*> TriRefList;

	// get the list of triangles that the object could have collided with this frame
	virtual void getTriList(const Point3& pos, const Point3& vel, TriRefList& list) = 0;

	// return true with u if collision occurred
	virtual bool collideTri(const Point3& pos, const Point3& vel, const Tri& t, float &u) = 0;

	typedef std::vector<CollisionInfo> Hits;

	// return the information about what the object hit last frame
	const Hits& lastHits()					{ return m_lastHits; }
	void clearHits()						{ m_lastHits.clear(); }
	void addHit(const CollisionInfo& hit)	{ m_lastHits.push_back(hit); }
	bool hasHitNormal(const Point3& normal, float tolerance = 0);

	TPDelegate<class CollisionInfo&> onCollide;

protected:
	virtual void levelChanging(Level* l);

	void			incAddRef();
	void			decAddRef();
	int				m_addRef;

	Point3			m_vel;
	Point3			m_angularVel;
	Point3			m_maxVel;
	Point3			m_impulse;
	float			m_mass;
	float			m_friction;
	float			m_gravityScale;
	float			m_elasticity;
	D3DMATERIAL9	m_material;
	Hits			m_lastHits;
	PtrGC<Object>	m_associatedObj;

	// collision groups
	friend class CollisionGroup;
	friend class PhysicsMgr;
	typedef stdext::hash_set<EmbeddedPtr<CollisionGroup> > Groups;
	Groups m_groups;

	typedef std::vector<CollisionInfo> Touching;
	Touching m_touching;
};

#endif
