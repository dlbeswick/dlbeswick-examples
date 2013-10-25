// ------------------------------------------------------------------------------------------------
//
// Head
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Head.h"
#include "PancakeLevel.h"
#include "Weapon.h"
#include "Man.h"
#include "RSE/Game/SpriteFX.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/Physics/PhysSphere.h"
#include "RSE/Physics/PhysicsMgr.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/ParticleSystem.h"
#include "Standard/Mapping.h"

IMPLEMENT_RTTI(Head);

Head::Head(const Point3& worldPos, float _pancakeValue, TextureAnimationSet& animationSet, bool squashed, Weapon* squashedWeapon) :
	Physical(animationSet),
	_worldSpawnPos(worldPos),
	_squashed(squashed),
	_squashedWeapon(squashedWeapon),
	pancakeValue(_pancakeValue)
{
}

void Head::createPhysics()
{
	_physics = new PhysSphere(0);
	_physics->setPos(_worldSpawnPos);
	_physics->setElasticity(0.75f);
	_physics->setFriction(4.0f);
}

void Head::constructed()
{
	Super::constructed();

	if (_squashed)
	{
		assert(_squashedWeapon);
		onSquash(*_squashedWeapon);
	}
	else
	{
		counters.set(2, delegate(&Head::stopBleed));
		
		sprite().animator().play("head", true);
		
		PhysSphere& sphere = *Cast<PhysSphere>(_physics);
		sphere.setRadius(size().z * 0.5f);
	}
}

void Head::update(float delta)
{
	Super::update(delta);

	if (!_squashed)
	{
		float angle = Mapping::linear(groundSpeed(), 0.0f, 50.0f, 0.0f, PI);
		if (physics().vel().x < 0)
			sprite().setRot(sprite().rot() * QuatAngleAxis(angle * delta, Point3(0, -1, 0)));
		else
			sprite().setRot(sprite().rot() * QuatAngleAxis(angle * delta, Point3(0, 1, 0)));
	}
}

void Head::onSquash(Weapon& w)
{
	_squashed = true;

	// must play here. Physical::onAnimationFrame sets the size of the object.
	sprite().animator().play(w.damageAnimationName(sprite().animator(), "headsquashed"));
	sprite().setRot(Quat::IDENTITY);

	PtrGC<PhysSphere> sphere = Cast<PhysSphere>(_physics);
	float oldRadius = sphere->radius();
	sphere->setRadius(size().z * 0.5f);
	sphere->setPos(sphere->pos() + Point3(0, 0, sphere->radius() - oldRadius));
	sphere->setFriction(20.0f);

	setRot(Quat::IDENTITY);
	_physics->setRot(Quat::IDENTITY);
	_physics->setAngularVel(Point3::ZERO);

	if (isIntersectingSideWalls())
	{
		if (w.man())
		{
			givePancakeToMan(*w.man());
		}
		else
		{
			_physics.destroy();
			self().destroy();
		}
	}
	else
	{
		_physics->onCollide = delegate(&Head::onCollide);
	}
}

void Head::stopBleed()
{
	sprite().animator()[0].spriteFX().stop();
}

void Head::onCollide(class CollisionInfo& info)
{
	PtrGC<Man> m;

	if (info.collider == _physics)
	{
		m = Cast<Man>(info.victim->associatedObj());
	}
	else
	{
		// tbd: find out why the collider can be null
		if (!info.collider)
			return;

		m = Cast<Man>(info.collider->associatedObj());
	}

	info.processHit = !m;

	if (m)
	{
		if (!m->canPickup())
		{
			return;
		}
		else
		{
			givePancakeToMan(*m);
		}
	}
}

void Head::givePancakeToMan(Man& m)
{
	m.onGotPancake(this);
	_physics.destroy();
	self().destroy();
}
