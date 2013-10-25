// ------------------------------------------------------------------------------------------------
//
// Projectile
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"

#include "Man.h"
#include "PancakeLevel.h"
#include "Projectile.h"
#include "Weapon.h"
#include "RSEApp.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/Physics/PhysicsMgr.h"
#include "RSE/Physics/PhysPlane.h"
#include "RSE/Render/TextureAnimationManager.h"
#include "RSE/Physics/PhysSphere.h"
#include "Standard/Config.h"

REGISTER_RTTI_ARG1(Projectile, WeaponRanged*);

Projectile::Projectile(WeaponRanged* weapon) :
	Super(App().textureAnimation().set(weapon->get<std::string>("ProjectileAnim")))
{
	_damage = weapon->damageAmt();
	_knockback = weapon->knockback();
	_attacker = weapon->man();
	_weaponID = weapon->weaponID();
	_weapon = weapon;

	counters.set(weapon->get("ProjectileKillTime", 1.0f), delegate(&Projectile::doBreak));
}

void Projectile::constructObject(Object& parent)
{
	Super::constructObject(parent);

	BEGIN_STATEMACHINE;
	ADD_STATE(Flying);
	ADD_STATE(Breaking);
	states.go(Flying);
}

void Projectile::createPhysics()
{
	_physics = new PhysSphere(_weapon->projectileSize);
	_physics->setParent(_attacker->level());
	_physics->setAssociatedObj(this);
	_physics->onCollide = delegate(&Projectile::onCollide);
	_physics->setGravityScale(_weapon->projectileGravity);
	_physics->setMass(_weapon->get("ProjectileMass", 1.0f));
	_physics->setElasticity(_weapon->get("ProjectileElasticity", 1.0f));
}

void Projectile::onCollide(CollisionInfo& info)
{
	states->onCollide(info);
}

void Projectile::update(float delta)
{
	Super::update(delta);
	states.update(delta);
}

void Projectile::doBreak()
{
	states.go(Breaking);
}

// States

void Projectile::StateFlying::start()
{
	o->sprite().animator().play("fly", true);
}
void Projectile::StateFlying::update(float delta)
{
}
void Projectile::StateFlying::onCollide(CollisionInfo& info)
{
	PtrGC<PhysicsObject> hitObj = info.other(&o->physics());
	if (!hitObj)
		return;

	if (hitObj == o->level().frontWall 
		|| hitObj == o->level().leftWall
		|| hitObj == o->level().rightWall)
	{
		info.processHit = false;
		return;
	}

	PtrGC<Physical> hit = Cast<Physical>(hitObj->associatedObj());
	if (hit == o->_attacker)
	{
		info.processHit = false;
		return;
	}

	if (hit)
	{
		Quat knockbackRot;
		if (o->physics().vel().x < 0)
			knockbackRot = Quat::IDENTITY;
		else
			knockbackRot = QuatAngleAxis(PI, Point3(0, 0, 1));

		const char* damageType;
		if (o->_weapon->projectileKnockdown)
			damageType = "projectileknockdown";
		else
			damageType = "";

		hit->onDamage(o->_attacker, o->_damage, o->_knockback * knockbackRot, AABB(), o->_weaponID, damageType);
	}

	o->doBreak();
}

void Projectile::StateBreaking::start()
{
	o->sprite().animator().play("break");
	o->setPos(o->worldPos());
	o->setParent(o->level().objectRoot());
	o->physics().removeFromGroups();
}
void Projectile::StateBreaking::update(float delta)
{
	if (o->sprite().animator().finished())
	{
		o->self().destroy();
	}
}
