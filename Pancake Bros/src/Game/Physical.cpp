// ------------------------------------------------------------------------------------------------
//
// Physical
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Physical.h"
#include "PancakeLevel.h"
#include "PancakeSprite.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/Physics/PhysicsMgr.h"
#include "RSE/Physics/PhysicsObject.h"
#include "RSE/Physics/PhysPlane.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/TextureAnimationTypes.h"
#include "Standard/Collide.h"

IMPLEMENT_RTTI(Physical);

#define METADATA Physical
STREAM
	STREAMVARDEFAULT(drawOffset, Point3(0, 0, 0));
}

Physical::Physical(TextureAnimationSet& animationSet) :
	_sprite(new PancakeSprite(animationSet)),
	_health(100),
	_bRespectBoundary(true)
{
	_sprite->setDrawScale(512.0f);
	_sprite->animator()[0].onFrame.add(*this, &Physical::onAnimationFrame);
};

Physical::~Physical()
{
	_physics.destroy();
}

void Physical::constructObject(Object& parent)
{
	setConfig(parent.level().cast<PancakeLevel>().configGame);
	Super::constructObject(parent);
	createPhysics();
	composite(*_physics);
	composite(*_sprite);
	_sprite->setDrawOffset(drawOffset);
}

void Physical::setLevel(Level* l)
{
	if (!l)
		clearConfig();

	Super::setLevel(l);
}

void Physical::setParent(Level& l)
{
	Super::setParent(l);
}

void Physical::setParent(const PtrGC<Object>& p)
{
	Super::setParent(p);

	_physics->removeFromGroups();
	if (hasLevel())
		level().collisionGroup().add(&physics());
}

void Physical::constructed()
{
	Super::constructed();
}

void Physical::update(float delta)
{
	Super::update(delta);

	if (level().options.get("Debug", "FootPlacement", false))
	{
		{
			DeferredPainter p(level());
			p->setFill(RGBA(1,1,1), 0, true);
			p->tick(worldPos(), 10);
			p->tick(worldPos() + Point3(0, 0, -physics().sphereBound()), 10);
		}
		{
			DeferredPainter p(level());
			p->setFill(RGBA(0.9f,0.75f,0.6f), 0, true);
			p->tick(Point3(worldPos().x, worldPos().y, level().boundMin().z), 10);
		}
	}
}

bool Physical::isInBoundary() const
{
	Point3 boundMin(level().boundMin());
	Point3 boundMax(level().boundMax());

	const Point3& p = pos();
	return p.x >= boundMin.x
		&& p.y >= boundMin.y
		&& p.z >= boundMin.z + _size.z * 0.5f
		&& p.x <= boundMax.x
		&& p.y <= boundMax.y
		&& p.z <= boundMax.z
	;
}

bool Physical::isIntersectingSideWalls() const
{
	Plane plane;
	
	if (Collide::spherePlane(_physics->worldPos(), _physics->sphereBound(), level().leftWall->plane()))
		return true;
	else if (Collide::spherePlane(_physics->worldPos(), _physics->sphereBound(), level().rightWall->plane()))
		return true;
	else
		return false;
}

float Physical::groundSpeed() const
{
	return Point2(_physics->vel().x, _physics->vel().y).length();
}

void Physical::onAnimationFrame(const AnimationFrame& frame)
{
	TextureAnimationFrame& texFrame = (TextureAnimationFrame&)frame;
	_size = Point3(texFrame.size().x, texFrame.size().x, texFrame.size().y) * 512.0f;
}

Point3 Physical::size() const
{
	// you must generally play an animation before size will return a valid value.
	assert(_size != Point3::MAX); 

	return _size;
}
	
float Physical::health() const 
{ 
	return _health; 
}

PancakeLevel& Physical::level() const 
{ 
	return Super::level().cast<PancakeLevel>(); 
}
