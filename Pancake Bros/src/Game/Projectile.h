// ------------------------------------------------------------------------------------------------
//
// Projectile
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Physical.h"
#include <Standard/StateMachine.h>


class Projectile : public Physical
{
	USE_RTTI(Projectile, Physical);
public:
	Projectile(class WeaponRanged* weapon);

	virtual void update(float delta);
	void doBreak();

protected:
	virtual void constructObject(Object& parent);

	// states
	EXTEND_STATECLASS(Projectile)
	{
		virtual void onCollide(class CollisionInfo& info) {};
	};

	DECLARE_STATEMACHINE;

	STATE(Flying)
	{
		void start();
		void update(float delta);
		void onCollide(class CollisionInfo& info);
	};

	STATE(Breaking)
	{
		void start();
		void update(float delta);
	};

	virtual void createPhysics();

	virtual void onCollide(class CollisionInfo& info);

	float _damage;
	Point3 _knockback;
	EmbeddedPtr<class WeaponRanged> _weapon;
	PtrGC<class Man> _attacker;
	std::string _weaponID;
};