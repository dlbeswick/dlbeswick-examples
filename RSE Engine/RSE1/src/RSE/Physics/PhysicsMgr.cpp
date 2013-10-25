// ------------------------------------------------------------------------------------------------
//
// PhysicsMgr
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "PhysicsMgr.h"
#include "PhysicsObject.h"
#include "PhysSphere.h"
#include "PhysPlane.h"
#include "PhysCylinderAA.h"
#include "PhysRay.h"
#include "Game/Database2D.h"
#include "Game/Level.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include <Standard/Config.h>
#include "Standard/Collide.h"
#include "Standard/Log.h"
#include "Standard/SteppableThreadHost.h"
#include "Standard/ThreadStepper.h"
#include "Standard/STLHelp.h"
#include "Terrain/TerrainDB.h"

#define LOGGING 0

// Collision functions
class CollisionFuncSwapAdapter : public PhysicsMgr::CollisionFunc
{
public:
	CollisionFuncSwapAdapter(PhysicsMgr::CollisionFunc* _func) :
	  func(_func)
	{
	}

	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		return (*func)(p1, p0, start0, end0, start1, end1, info1, info0);
	}
	
	PhysicsMgr::CollisionFunc* func;
};

class SphereSphere : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		PhysSphere* o0 = (PhysSphere*)p0.ptr();
		PhysSphere* o1 = (PhysSphere*)p1.ptr();

		float u = FLT_MAX;
		if (Collide::sphereSphereSweep(start0, end0, o0->radius(), start1, end1, o1->radius(), u))
		{
			Point3 pAtHit0 = start0 + (end0 - start0) * u;
			Point3 pAtHit1 = start1 + (end1 - start1) * u;
			Point3 p0p1 = pAtHit1 - pAtHit0;
			p0p1.normalise();

			info0.normal = -p0p1;
			info0.u = u;
			info0.point = pAtHit0 + p0p1 * o0->radius();
			info1.normal = p0p1;
			info1.u = u;
			info1.point = info0.point;

			return true;
		}

		return false;
	}
};

class SpherePlane : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		PhysSphere* o0 = (PhysSphere*)p0.ptr();
		PhysPlane* o1 = (PhysPlane*)p1.ptr();

		Plane plane(o1->normal(), o1->pos());
		float u;
		if (Collide::spherePlaneSweep(start0, end0, o0->radius(), plane, u))
		{
			Point3 pAtHit0 = start0 + (end0 - start0) * u;

			info0.normal = o1->normal();
			info0.u = u;
			info0.point = pAtHit0 - plane.n * plane.distance(pAtHit0);
			info0.friction = -o0->vel().normal();
			info0.friction -= plane.n * plane.distance(info0.friction);
			info0.friction.normalise();
			info1.normal = o1->normal();
			info1.u = u;
			info1.point = info0.point;

			return true;
		}

		return false;
	}
};

class CylinderAAPlane : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		PhysCylinderAA* o0 = (PhysCylinderAA*)p0.ptr();
		PhysPlane* o1 = (PhysPlane*)p1.ptr();

		Plane plane(o1->normal(), start0);
		if (Collide::discAAPlane(start0, o0->radius(), /*o0->height(), */plane))
		{
			o0->setVel(o0->vel().rComponentMul(Point3(1, 1, 0)));
			info0.normal = o1->normal();
			info0.u = 0;
			info1.normal = o1->normal();
			info1.u = 0;

			return true;
		}

		return false;
	}
};

class RayPlane : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		PhysRay* o0 = (PhysRay*)p0.ptr();
		PhysPlane* o1 = (PhysPlane*)p1.ptr();

		if (Collide::rayPlaneDist(o0->pos(), o0->dir(), Plane(o1->normal(), o1->pos()), info0.u))
		{
			o0->setVel(o0->vel().rComponentMul(Point3(1, 1, 0)));
			info0.normal = -o0->dir();
			info1.normal = info0.normal;
			info1.u = 0;

			return true;
		}

		return false;
	}
};

class RaySphere : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		PhysRay* o0 = (PhysRay*)p0.ptr();
		PhysSphere* o1 = (PhysSphere*)p1.ptr();

		if (Collide::raySphere(o0->pos(), o0->dir(), o1->pos(), o1->radius()))
		{
			//info0.normal = info0.point - o1->pos();
			//info0.u = (info0.point - o0->pos()).length();
			//info1.normal = info0.normal;
			//info1.u = 0;

			return true;
		}

		return false;
	}
};

class RayCylinderAA : public PhysicsMgr::CollisionFunc
{
public:
	bool operator() (const PtrGC<class PhysicsObject>& p0, const PtrGC<class PhysicsObject>& p1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1) const
	{
		//PhysRay* o0 = (PhysRay*)p0.ptr();
		//PhysCylinderAA* o1 = (PhysCylinderAA*)p1.ptr();

		/*if (Collide::rayCylinderAA(o0->pos(), o0->dir(), o1->pos(), o1->height(), o1->radius(), info0.point))
		{
			o0->setVel(o0->vel().rComponentMul(Point3(1, 1, 0)));
			info0.normal = -o0->dir();
			info0.u = (info0.point - o0->pos()).length();
			info1.normal = info0.normal;
			info1.u = 0;

			return true;
		}*/

		return false;
	}
};

void PhysicsMgr::addCollisionHandler(const RTTI* lhs, const RTTI* rhs, CollisionFunc* func)
{
	m_collisionMap[lhs][rhs] = func;
	if (lhs != rhs)
		m_collisionMap[rhs][lhs] = new CollisionFuncSwapAdapter(func);
}

PhysicsMgr::PhysicsMgr(Level& l) :
	m_level(l),
	m_accumulator(0),
	m_useCollisions(true)
{
	addCollisionHandler(&PhysSphere::static_rtti(), &PhysSphere::static_rtti(), new SphereSphere);
	addCollisionHandler(&PhysSphere::static_rtti(), &PhysPlane::static_rtti(), new SpherePlane);
	addCollisionHandler(&PhysCylinderAA::static_rtti(), &PhysPlane::static_rtti(), new CylinderAAPlane);
	
	// rays
	addCollisionHandler(&PhysRay::static_rtti(), &PhysPlane::static_rtti(), new RayPlane);
	addCollisionHandler(&PhysRay::static_rtti(), &PhysSphere::static_rtti(), new RaySphere);
	addCollisionHandler(&PhysRay::static_rtti(), &PhysCylinderAA::static_rtti(), new RayCylinderAA);

	m_updateFreq = AppBase().options().get<int>("Physics", "UpdateFreq", 60);
	m_maxSteps = 32;
	m_maxDynamicSteps = 16;
}

PhysicsMgr::~PhysicsMgr()
{
	// FIX THIS
	/*for (CollisionMap::iterator i = m_collisionMap.begin(); i != m_collisionMap.end(); ++i)
	{
		freeHash(i->second);
	}*/
}

void fixDenorm(float& f)
{
	if (f < 0 && f > -1.0e-6)
		f = 0;
	if (f > 0 && f < 1e-6)
		f = 0;
}

// update
void PhysicsMgr::update(float delta)
{
	if (!m_updateFreq)
	{
		step(delta);
	}
	else
	{
		float fixedDelta = 1.0f / m_updateFreq;

		m_accumulator += delta;
		while (m_accumulator >= fixedDelta)
		{
			step(fixedDelta);
			m_accumulator -= fixedDelta;
		}
	}
}

void PhysicsMgr::step(float delta)
{
	CollisionInfo collisionInfo0;
	CollisionInfo collisionInfo1;

	m_objects.flush();

	bool bDidCollide;
	bool log = AppBase().options().get("Debug", "LogPhysics", false);
	int step = 0;

	do
	{
		HitRecords hits;

		clearVis();

		STEPMARK(AppBase().mainLoopStep(), "Physics Step");

		if (log) dlog << "Step " << step << dlog.endl;

		bDidCollide = false;
 
		// fix: wish i didn't have to iterate over all objects here
		for (Objects::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
		{
			PtrGC<PhysicsObject>& o0 = *i;

			o0->m_touching.clear();

			if (step == 0 && !o0->immovable())
			{
				// apply impulse and gravity to objects
				o0->clearHits();
				o0->setVel(o0->vel() + o0->impulse() + m_level.gravity() * o0->gravityScale() * delta);
				o0->setImpulse(Point3::ZERO);
				clampVelocity(*o0);
			}
		}

		for (CollisionGroups::iterator gi = m_groups.begin(); gi != m_groups.end(); ++gi)
		{
			CollisionGroup& g = **gi;

			if (step == 0)
			{
				g.m_objects.flush();
			}

			if (m_useCollisions)
			{
				for (Objects::iterator i = g.m_objects.begin(); i != g.m_objects.end(); ++i)
				{
					PtrGC<PhysicsObject>& o0 = *i;

					if (!o0->immovable())
					{
						// clear touching records
						o0->m_touching.clear();

						HitRecords r;
						findCollisions(g, o0, delta, step, r);
						if (o0 && !r.empty())
						{
							for (uint i = 0; i < r.size(); ++i)
							{
								HitRecord& hit = r[i];

								o0->m_touching.push_back(hit);

								if (hit.result == CollisionInfo::HIT)
								{
                    				hits.push_back(hit);
									if (log) dlog << "HIT: " << hit.collider->name() << " hit " << hit.victim->name() << " N:" << hit.normal << " P:" << hit.point << " U:" << hit.u << dlog.endl;
								}
							}
						}
					}
				}
			}
		}

		// find time of first collision
		float firstHitTime = 0;
		float stepDelta = delta;

		bDidCollide = !hits.empty();
		if (bDidCollide)
		{
			// sort list in order of hit time
			std::sort(hits.begin(), hits.end());

			firstHitTime = hits.front().u;
			//stepDelta *= std::max(0.0f, firstHitTime - 0.000001f);
			stepDelta *= firstHitTime;
		}

		// integrate to collision point
		if (stepDelta > 0)
		{
			for (Objects::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
			{
				PtrGC<PhysicsObject>& o0 = *i;
				if (o0->immovable())
					continue;

				// if collisions not enabled, impulse and gravity will not have been applied
				if (!m_useCollisions)
				{
					// apply impulse and gravity to objects
					o0->clearHits();
					o0->setVel(o0->vel() + o0->impulse() + m_level.gravity() * o0->gravityScale() * delta);
					o0->setImpulse(Point3::ZERO);
					clampVelocity(*o0);
				}
				o0->setPos(o0->pos() + o0->vel() * stepDelta);
				Quat newRot = Quat(0.0f, o0->angularVel()) * o0->rot() * 0.5f * stepDelta;
				newRot += o0->rot();
				newRot.normalise();
				o0->setRot(newRot);
			}
		}
		STEPMARK(AppBase().mainLoopStep(), "Phys. collision point integrated");

		delta -= stepDelta;

		// resolve collisions
		HitResolutions resolutions;
		for (HitRecords::iterator i = hits.begin(); i != hits.end(); ++i)
		{
			CollisionInfo& info0 = *i;

			// only resolve hits up to first hit time
			if (info0.u > firstHitTime)
				break;

			// objects may have been destroyed during collision response
			PhysicsObject* o0 = info0.collider.ptr();
			PhysicsObject* o1 = info0.victim.ptr();

			// fix: may be inaccurate if obj destroyed no collision response
			if (!o0 || !o1)
				break;

			m_visHit.push_back(info0);

			// calculate collision response
			collisionResponse(info0, resolutions, delta, step);

#if LOGGING
			dlog << "START " << " Pos0: " << o->pos() << " Vel0: " << o->vel() << " Pos1: " << collider->pos() << " Vel1: " << collider->vel() << " U: " << hit->u << " Normal: " << hit->normal << "\n";
#endif
			// avoid denormalisation
			/*fixDenorm(vel0.x);
			fixDenorm(vel0.y);
			fixDenorm(vel0.z);
			fixDenorm(vel1.x);
			fixDenorm(vel1.y);
			fixDenorm(vel1.z);*/

			o0->addHit(info0);

#if LOGGING
			dlog << "END " << " Pos: " << o->pos() << " Vel: " << o->vel() << "\n";
#endif

			//m_visHitPoly.push_back(first.t);
		}

		// apply collision resolutions
		for (uint i = 0; i < resolutions.size(); ++i)
		{
			HitResolution& r = resolutions[i];
			PhysicsObject& o = *r.subject;
			Point3 newVel = o.vel() + r.impulse;
			o.setVel(newVel);

			if (log) dlog << "Resolve: " << r.subject->name() << " vel + " << r.impulse << " = " << r.subject->vel() << dlog.endl;
			
			clampVelocity(o);
		}

		++step;
	}
	while (bDidCollide && step < m_maxSteps && delta > 0);

	if (step == m_maxSteps)
		STEPMARK(AppBase().mainLoopStep(), "Physics Done (max steps exceeded)");
	else
		STEPMARK(AppBase().mainLoopStep(), "Physics Done");
}

CollisionInfo::HitResult PhysicsMgr::detectCollision(const PtrGC<PhysicsObject>& o0, const PtrGC<PhysicsObject>& o1, const Point3& start0, const Point3& end0, const Point3& start1, const Point3& end1, CollisionInfo& info0, CollisionInfo& info1)
{
	CollisionInfo::HitResult result = CollisionInfo::NOHIT;

	if (!o0 || !o1)
		return result;

	CollisionFunc* c = m_collisionMap[&o0->rtti()][&o1->rtti()];
	if (c && (*c)(o0, o1, start0, end0, start1, end1, info0, info1))
	{
		result = CollisionInfo::HIT;

		info0.collider = o0.ptr();
		info0.victim = o1.ptr();
		info1.victim = o1.ptr();
		info1.collider = o0.ptr();

		// if objects are making contact but are moving away from each other or in the same direction, then discard the hit
		// this is better than moving the objects slight apart on collision resolution.
		// if the objects were just an atom apart and the victim object were moving equal to or faster than the
		// collider object, collision would never occur.
		if (result == CollisionInfo::HIT && info0.u == 0)
		{
			float dot0 = info0.collider->vel() * info0.normal;

			if (dot0 >= 0)
			{
				result = CollisionInfo::CONTACT;
			}
			else
			{
				float dot1 = info0.victim->vel() * info0.normal;

				if (dot0 > 0 && dot1 < 0)
				{
					// moving apart
					result = CollisionInfo::CONTACT;
				}
				else
				{
					if ((dot0 <= 0 && dot1 < 0) || (dot0 > 0 && dot1 >= 0))
					{
						// moving in same direction, victim at greater or equal speed
						if (abs(dot0) <= abs(dot1))
							result = CollisionInfo::CONTACT;
					}
				}
			}
		}
	}

	if (result == CollisionInfo::HIT)
	{
		if (o0->vel() * info0.normal >= 0)
		{
			result = CollisionInfo::NOHIT;
		}
		else
		{
			o0->onCollide(info0);
			o1->onCollide(info1);

			if (!info0.processHit || !info1.processHit)
				result = CollisionInfo::NOHIT;
		}
	}

	info0.result = result;
	info1.result = result;

	return result;
}

// drawDebug
void PhysicsMgr::drawDebug()
{
	D3D().zbuffer(true);

	for (uint i = 0; i < m_visHit.size(); i++)
	{
		CollisionInfo& info = m_visHit[i];

		D3DPaint().setFill(RGBA(0,0,1));

		Quat q = QuatFromVector(Point3(0, 0, 1), info.normal);

		D3DPaint().line(info.point, info.point + info.normal * 20);
		D3DPaint().line(info.point, info.point + (Point3(1, 0, 0) * q) * 10);
		D3DPaint().line(info.point, info.point + (Point3(0, 1, 0) * q) * 10);
	}
	D3DPaint().draw();

	for (uint i = 0; i < m_visHitPoly.size(); i++)
	{
		D3DPaint().setFill(RGBA(1,0,0));
		D3DPaint().tri(*m_visHitPoly[i]);
	}
	D3DPaint().draw();
}

// clearVis
void PhysicsMgr::clearVis()
{
	m_visHit.clear();
	m_visHitPoly.clear();
}


PhysicsMgr::HitRecord PhysicsMgr::findFirstCollision(const PtrGC<PhysicsObject>& o0)
{
	HitRecords hits;

	for (CollisionGroups::iterator gi = m_groups.begin(); gi != m_groups.end(); ++gi)
	{
		CollisionGroup& g = **gi;
		findCollisions(g, o0, 0, 0, hits); // fix, put delta, I'm lazy
	}

	if (!hits.empty())
	{
		std::sort(hits.begin(), hits.end());
		return hits[0];
	}

	return HitRecord();
}

void PhysicsMgr::findCollisions(CollisionGroup& g, const PtrGC<PhysicsObject>& o0, float delta, int step, HitRecords& hits)
{
	CollisionInfo t0, t1;
	Point3 dest0 = o0->pos() + o0->vel() * delta;

	if (step < m_maxDynamicSteps)
	{
		for (Objects::iterator i = g.m_objects.begin(); i != g.m_objects.end(); ++i)
		{
			if (!o0)
				return;

			PhysicsObject* o1 = i->ptr();
			if (o0 == o1)
				continue;

			Point3 dest1 = o1->pos();

			if (!o1->immovable())
			{
				dest1 += o1->vel() * delta;
			}

			if (detectCollision(o0, *i, o0->pos(), dest0, o1->pos(), dest1, t0, t1) != CollisionInfo::NOHIT)
			{
				hits.push_back(t0);
			}
		}
	}
}

EmbeddedPtr<CollisionGroup> PhysicsMgr::addCollisionGroup()
{
	m_groups.push_back(new CollisionGroup(m_level));
	return m_groups.back();
}

void PhysicsMgr::collisionResponse(CollisionInfo& info, HitResolutions& resolutions, float delta, int step)
{
	PhysicsObject& collider = *info.collider;
	PhysicsObject& victim = *info.victim;

	if (victim.immovable())
	{
		Point3 vel0 = collider.vel();
		applyFriction(vel0, info.normal, info.friction, collider.friction(), delta);
		Point3 friction = vel0 - collider.vel();

		float colFrameVel0 = vel0 * -info.normal;

		// apply elasticity
		if (collider.elasticity())
		{
			// hack to avoid inelastic objects jittering when resting on the ground.
			// jittering occurs on large frame deltas.
			// gravity contributes an amount of velocity proportionate to the frame delta.
			// when the frame delta is large, gravity contributes a large velocity to the object.
			// this will occur regardless of the collider's position relative to the collidee.
			// so if a rubber ball is close to the floor, a situation which would not result in a
			// large rebound in reality, a large rebound may occur with large frame deltas because a lot
			// of velocity has been contributed by gravity, and the amount of rebound is calculated by the
			// elasticity variable and the object's velocity.
			// the hack removes gravity's contribution to the collision frame velocity, meaning that objects
			// resting on other immovable objects won't jitter or rebound based on the frame delta.
			// note that the hack ensures that collisions no occuring along the velocity vector of gravity
			// aren't affected, such as a ball hitting the wall.
			// the hack works because for most objects, the contribution of the current frame's gravity is small 
			// compared to the object's existing velocity. the result of the collision resolution is
			// inaccurate, but the inaccuracy is too insignificant to be noticed.
			Point3 gravityVelAddedThisFrame = m_level.gravity() * delta;
			float thisFrameGravityAlongHitNormal = gravityVelAddedThisFrame * -info.normal;

			// i arrived at this result after guessing the correct resolution intuitively, then simplifying
			// the resulting equation.
			// it turns out the solution for what i was trying to achieve is a quadratic equation.
			//
			// collision_response_impulse_along_hit_normal_sans_gravity = 
			//  -delta_frame_gravity * elasticity^2 + velocity_along_collision_normal * elasticity + velocity_along_collision_normal
			//
			// i don't know what that signifies.
			colFrameVel0 = (-thisFrameGravityAlongHitNormal * DMath::sqr(collider.elasticity())) + (colFrameVel0 * collider.elasticity()) + colFrameVel0;
		}

		resolutions.push_back(HitResolution(info.collider, friction + info.normal * colFrameVel0));
	}
	else
	{
		// no friction or elasticity on dynamic-dynamic for now
		//applyFriction(vel0, info.normal, collider.friction(), delta);

		float inverseMasses = 1.0f / collider.mass() + 1.0f / victim.mass();
		float e = 0.0f;
		float impulse = -(1.0f + e) * ((collider.vel() - victim.vel()) / inverseMasses) * info.normal;

		Point3 finalVelCollider = collider.vel() + impulse / collider.mass() * info.normal;
		Point3 finalVelVictim = victim.vel() - impulse / victim.mass() * info.normal;

		// quick solution -- there may be a better one that permits stacking, but this is all we need for the time being
		/*for (uint i = 0; i < victim.m_touching.size(); ++i)
		{
			CollisionInfo& touchInfo = victim.m_touching[i];
			if (touchInfo.victim->immovable())
			{
				float impulse = collider.vel() * touchInfo.normal;
				resolutions.push_back(HitResolution(info.collider, -touchInfo.normal * impulse));
			}
		}*/

		resolutions.push_back(HitResolution(info.collider, finalVelCollider - collider.vel()));
		resolutions.push_back(HitResolution(info.victim, finalVelVictim - victim.vel()));
	}

	STEPMARK(AppBase().mainLoopStep(), "Collision Response");
}

void PhysicsMgr::propagateMomentum(CollisionInfo& info, float momentum, HitResolutions& resolutions)
{
	if (momentum == 0)
		return;

	PhysicsObject& victim = *info.victim;
	PhysicsObject& collider = *info.collider;

	// apply impulse to hit object if required
	if (!victim.immovable())
	{
		// convert momentum to impulse if we are a terminal object
		// terminal objects have no moveable touchers to which impulse can be applied, or all immovable touchers have 
		// returned momentum to the terminal object
		bool shouldApplyImpulse = true;
		for (uint i = 0; i < victim.m_touching.size(); ++i)
		{
			CollisionInfo& touchInfo = victim.m_touching[i];
			Point3 toVictim = touchInfo.victim->pos() - victim.pos();
			if (toVictim * touchInfo.normal > 0 && (!touchInfo.victim->immovable() || touchInfo.momentumPropagated))
			{
				shouldApplyImpulse = false;
				return;
			}
		}

		if (shouldApplyImpulse)
		{
			Point3 impulse = -info.normal * (momentum / victim.mass());
			resolutions.push_back(HitResolution(info.victim, impulse));

			// no more processing required
			return;
		}
	}
	else
	{
		// a movable object is propagating momentum to an immovable object
		// the immovable object will push back along the hit normal with the same momentum with which it was hit
		info = info.mirrored();
		info.momentumPropagated = true;
		propagateMomentum(info, momentum, resolutions);
		return;
	}

	// have victim transfer impulses to objects that it touches
	float normalizedMomentum = momentum / victim.m_touching.size();
	for (uint i = 0; i < victim.m_touching.size(); ++i)
	{
		CollisionInfo& touchInfo = victim.m_touching[i];
		// don't propagate back to propagating object
		// don't propagate to objects facing away from the direction of effect
		// don't propagate back to immovable objects who have already propagated
		Point3 toCollider = touchInfo.collider->pos() - victim.pos();
		if (toCollider * touchInfo.normal > 0 && touchInfo.victim != &collider && !touchInfo.momentumPropagated)
		{
			touchInfo.momentumPropagated = info.momentumPropagated;
			propagateMomentum(touchInfo, normalizedMomentum, resolutions);
		}
	}
}

void PhysicsMgr::applyFriction(Point3& vel, const Point3& normal, const Point3& friction, float coefficient, float delta)
{
	if (friction == Point3::ZERO)
		return;

	// do friction calc
	float damping = coefficient * delta;
	float maxFriction = vel * -friction;
	float frictionForce = vel * -normal * damping;
	
	if (abs(frictionForce) > abs(maxFriction))
		frictionForce = maxFriction;
	
	Point3 frictionVec = friction * frictionForce;

	vel += frictionVec;
}

void PhysicsMgr::remove(const PtrGC<PhysicsObject>& o)
{
	for (uint i = 0; i < m_groups.size(); ++i)
	{
		m_groups[i]->remove(o);
	}
}

void PhysicsMgr::clampVelocity(PhysicsObject& o)
{
	// fix: move this, could be more efficient elsewhere
	{
	const Point3& maxVel = o.maxVel();
	if (maxVel != Point3::MAX)
	{
		Point3 vel = o.vel();
		if (maxVel.x != FLT_MAX)
			clamp(vel.x, -maxVel.x, maxVel.x);
		if (maxVel.y != FLT_MAX)
			clamp(vel.y, -maxVel.y, maxVel.y);
		if (maxVel.z != FLT_MAX)
			clamp(vel.z, -maxVel.z, maxVel.z);
		o.setVel(vel);
	}
	}
}

void PhysicsMgr::setUseCollisions(bool b)
{
	m_useCollisions = b;
}

void PhysicsMgr::clear() 
{ 
	m_groups.clear(); 
	m_objects.clear(); 
}
