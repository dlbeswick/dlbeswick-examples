// ------------------------------------------------------------------------------------------------
//
// Physical
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/ObjectComposite.h"


class Physical : public ObjectComposite
{
	USE_RTTI(Physical, ObjectComposite);
	USE_STREAMING;
public:
	Physical(class TextureAnimationSet& animationSet);
	virtual ~Physical();

	class PancakeLevel& level() const;
	virtual void setLevel(Level* l);
	virtual void update(float delta);

	const class SpriteLayered& sprite() const { return *_sprite; }
	class SpriteLayered& sprite() { return *_sprite; }

	virtual bool canBeSquashed() const { return false; }
	virtual float groundSpeed() const;
	virtual float health() const;
	virtual bool isInBoundary() const;
	virtual bool isIntersectingSideWalls() const;
	virtual void onDamage(const PtrGC<class Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType) {};
	virtual void onSquash(class Weapon& w) {};
	virtual Point3 size() const;

	class PhysicsObject& physics() const { return *_physics; }

	virtual void setParent(const PtrGC<Object>& p);
	virtual void setParent(class Level& l);

protected:
	virtual void constructObject(Object& parent);
	virtual void constructed();
	virtual void createPhysics() = 0;

	void onAnimationFrame(const class AnimationFrame& frame);

	Point3 _size;
	Point3 drawOffset;
	float _health;
	bool _bRespectBoundary;

	PtrGC<class PhysicsObject> _physics;
	PtrGC<class SpriteLayered> _sprite;
};