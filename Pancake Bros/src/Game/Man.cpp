// ------------------------------------------------------------------------------------------------
//
// Man
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Man.h"
#include "RSEApp.h"
#include "Game.h"
#include "PancakeLevel.h"
#include "Hammer.h"
#include "Weapon.h"
#include "Head.h"
#include "AI/AI.h"
#include "RSE/Game/SpriteFX.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"
#include "RSE/Physics/PhysicsMgr.h"
#include "RSE/Physics/PhysSphere.h"
#include "RSE/Physics/PhysCylinderAA.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/TextureAnimationManager.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/TextureAnimationTypes.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/SDeviceD3D.h"
#include "Standard/Collide.h"
#include "Standard/Mapping.h"
#include "Standard/MathHelp.h"
#include "Standard/STLHelp.h"
#include "Standard/Config.h"
#include "Standard/Profiler.h"

IMPLEMENT_RTTI(Man);
IMPLEMENT_RTTI(PlayerMan);
IMPLEMENT_RTTI(BaddyMan);


Man::Man(const std::string& characterType) :
	Physical(App().textureAnimation().set("")),
	_bEnemy(false),
	_bDesiredLeft(true),
	_bPlayedDeath(false),
	_aiIndicator(0),
	_weapon(0),
	_jumping(false)
{
	// allow config override based on character type name
	setConfigCategory("character " + characterType);

	_bHead = true;

	zero(_controls);
}

void Man::createPhysics()
{
	// ensure animations are set up before creating physics, so that _size is set
	// tbd: just access the animation, no need to set it yet
	std::string animSetName = get<std::string>("AnimationSet");

	sprite().animator().useSet(App().textureAnimation().set(animSetName));
	if (sprite().animator().set().name == "")
		throwf("No animation set '" + animSetName + "' found for character type " + configCategory());

	sprite().animator().play("rest", true);

	_size(sprite().animator().frame().size().x, sprite().animator().frame().size().x, sprite().animator().frame().size().y);
	_size *= (float)App().textureAnimation().texSize();

	float radius;
	get("CollisionRadius", radius);

	if (radius)
		_physics = new PhysSphere(radius);
	else
		_physics = new PhysSphere(_size.z / 2);

	//_physics = new PhysCylinderAA(_size);
	_physics->setMass(get<float>("Mass"));
	_physics->setAssociatedObj(this);
	_physics->onCollide = delegate(&Man::onCollide);
}

void Man::configLoad()
{
#if _DEBUG
	assert(!_constructed);
#endif

	const std::string& weaponName = get<std::string>("Weapon");
	_weapon = (Weapon*)Weapon::newObject(config().get<std::string>(weaponName, "Class"), weaponName);
	_weapon->setMan(this);

	get("Acceleration", _acceleration);
	get("ConcertinaPossible", _concertinaPossible);
	get("ControlAir", _controlAir);
	get("ControlAirAttack", _controlAirAttack);
	get("ControlRoll", _controlRoll);
	get("ChestArea", _chestArea);
	get("CrushingFeet", _crushingFeet);
	get("FakeShadowAdjustment", _fakeShadowAdjustment);
	get("Friction", _friction);
	get("KnockdownKnockbackFactor", _knockdownKnockbackFactor);
	get("KnockdownPossibleJumpAttack", _knockdownPossibleJumpAttack);
	get("KnockdownPossibleRoll", _knockdownPossibleRoll);
	get("RollFriction", _rollFriction);
	get("HeadArea", _headArea);
	get("HeadAttackAccel", _headAttackAccel);
	get("Health", _health);
	get("JumpAccel", _jumpAccel);
	get("JumpedOnHeadImpulse", _jumpedOnHeadImpulse);
	get("LegArea", _legArea);
	get("MaxSpeed", _maxSpeed);
	get("RollKnockback", _rollKnockback);
	get("RollSpeedFraction", _rollSpeedFraction);
	get("SquashExtent", _squashExtent);
	get("KnockdownTime", _knockdownTime);
	get("SquashCautionRadius", _squashCautionRadius);

	_maxHealth = _health;
}

void Man::constructObject(Object& parent)
{
	Super::constructObject(parent);

	BEGIN_STATEMACHINE;
	ADD_STATE(Move);
	ADD_STATE(MeleeAttack);
	ADD_STATE(RangeAttack);
	ADD_STATE(Squash);
	ADD_STATE(Roll);
	ADD_STATE(Hit);
	ADD_STATE(Dead);
	ADD_STATE(Knockdown);

	reset();
}

Man::~Man()
{
	if (_weapon)
		delete _weapon;
}

void Man::input(const ControlBlock& c)
{
	_controls = c;
}

void Man::update(float delta)
{
	Physical::update(delta);

	states.update(delta);

	if (!_bEnemy)
		ProfileValue("Man Speed", _physics->vel().length());

	zero(_controls);
}

bool Man::didJumpOnHead(const CollisionInfo& info) const
{
	if (info.normal.z < 0.5f)
		return false;

	return isAbove(*info.victim);
}

void Man::onCollide(CollisionInfo& info)
{
	if (info.victim == physics())
		return;

	float dot = info.normal * Point3(0, 0, 1);
	bool standing = dot >= 0.1f;

	if (_jumping && standing)
		_jumping = false;

	PtrGC<Man> m = Cast<Man>(info.victim->associatedObj());

	if (m)
	{
		if (didJumpOnHead(info))
		{
			if (m->states->onHadHeadJumpedOn())
				states->onJumpedOnHead(*m);
		}
		else if (didCrush(*m))
		{
			m->states.go(m->Knockdown);
		}
	}

	states->onCollide(info);
}

bool Man::didCrush(Man& victim) const
{
	return crushingFeet() 
		&& !victim.crushingFeet() 
		&& victim.canKnockDown() 
		&& !victim.isAbove(*physics())
		&& !victim.ignoresHits();
}

void Man::draw()
{
	Point3 p = worldPos();

	// position sprite with bottom of sprite (feet) at 0,0,0
	sprite().setDrawOffset(Point3(0, 0, (abs(_physics->extent().z - _size.z) / 2) + (_size.z * _fakeShadowAdjustment)));

	// draw main sprite
	Super::draw();

	// visualization
	if (level().options.get<bool>("Debug", "ShowPhysicsDebug"))
	{
		D3D().wireframe(true);
		
		D3DPaint().setFill(RGBA(0,1,0));
		D3DPaint().aabb(_lastHit);
		D3DPaint().draw();

		Matrix4 m;
		m.identity();
		m.translation(worldPos());
		OBB obb;
		obb.extent = size();
		obb.rot = Quat(1, 0, 0, 0);
		D3DPaint().setFill(RGBA(1,0,0));
		D3DPaint().obb(obb, m);
		D3DPaint().draw();
		
		if (*states == MeleeAttack)
		{
			WeaponMelee* w = Cast<WeaponMelee>(weapon());
			Point3 playerDir = rot().vec(Point3(-1, 0, 0));

			m.identity();
			m.translation(attackOrigin());
			OBB obb;
			if (!airborne())
			{
				obb.extent = w->attackExtent();
			}
			else
			{
				obb.extent = w->airAttackExtent();
			}
			obb.rot = Quat(1, 0, 0, 0);
			D3DPaint().setFill(RGBA(1,0,0));
			D3DPaint().obb(obb, m);
			D3DPaint().draw();
		}

		D3D().wireframe(false);
	}
}

const Point3& Man::squashExtent() const
{
	return _squashExtent;
}

PtrGC<Physical> Man::findSquashTarget()
{
	if (!_weapon)
		return 0;

	PtrGC<Physical> best = 0;
	float bestScore = -FLT_MAX;

	AABB weaponBox(worldPos(), squashExtent());

	const Level::ObjectList& c = level().autoList<Physical>();
	for (Level::ObjectList::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		PtrGC<Physical> p = Cast<Physical>(*i);
		if (p == this)
			continue;
		
		AABB targetBox(p->worldPos(), p->extent());

		// abort if any non-dead men are within radius
		PtrGC<Man> man = Cast<Man>(p);
		if (man && !man->dead() && !man->knockedDown())
		{
			if ((Point2(man->worldPos().x, man->worldPos().y) - Point2(worldPos().x, worldPos().y)).length() < _squashCautionRadius)
				return 0;
		}

		if (p->canBeSquashed())
		{
			float score = FLT_MAX - (p->pos() - pos()).sqrLength();
			if (score > bestScore && Collide::aabbAABB(weaponBox, targetBox))
			{
				best = p;
				bestScore = score;
			}
		}
	}

	return best;
}

void Man::onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType)
{
	if (ignoresHits())
		return;

	_lastHitWeapon = weaponString;
	_lastHit = localHit;
	if (*states)
		states->onDamage(attacker, amt, knockback, localHit, weaponString, damageType);
}

Point3 Man::attackOrigin()
{
	return weapon()->attackOrigin();
}

void Man::onSquash(Weapon& w)
{
	_bHead = false;
	sprite().animator().play(w.damageAnimationName(sprite().animator(), "headsquashedbody"));
	_headSquashedWeapon = &w;
}

void Man::setFacing(bool bLeft)
{
	_bDesiredLeft = bLeft;
}

void Man::updateFacing()
{
	states->updateFacing();
}

void Man::setAIIndicator(const std::string& name)
{
	if (!_aiIndicator)
	{
		_aiIndicator = new Sprite(App().textureAnimation().set("ai states"));
		_aiIndicator->setParent(level());
		_aiIndicator->setDrawScale(sprite().drawScale());
	}

	_aiIndicator->animator().play(name, true);
	_aiIndicator->setPos(Point3(0, 0, size().z * 0.5f + _aiIndicator->size().z * 0.5f));
}

void Man::onProjectileLaunchRequest(const Point2& pos)
{
	if (*states)
		states->onProjectileLaunchRequest(Point3(pos.x, 0, pos.y) * worldXForm());
}

void Man::onMeleeHitRequest()
{
	if (*states)
		states->onMeleeHitRequest();
}

void Man::onHeadSquashRequest(const Point2& pos)
{
	makeHead(Point3(pos.x, 0, pos.y), Point3(0, 0, 0), true);
}

bool Man::airborne() const
{
	return _physics && !_physics->hasHitNormal(Point3(0, 0, 1), 0.9f);
}

bool Man::running() const
{
	return abs(_physics->vel().x) > 0.01f;
}

bool Man::attacking() const
{
	return states->isAttacking();
}

bool Man::ignoresHits() const 
{ 
	return false;
}

void Man::processMovement(float delta, float scale)
{
	Point3 impulse(Point3::ZERO);
	Point3 accel(Point3::ZERO);

	updateFacing();

	bool left = false;
	bool right = false;

	if (_controls.left && _controls.right)
	{
		left = !_lastWasLeft;
		right = !left;
	}
	else if (_controls.left)
	{
		left = true;
		right = false;
		_lastWasLeft = true;
	}
	else if (_controls.right)
	{
		left = false;
		right = true;
		_lastWasLeft = false;
	}

	if (left)
	{
		accel.x -= _controls.left * _acceleration.x;
	}
	else if (right)
	{
		accel.x += _controls.right * _acceleration.x;
	}

	if (_controls.up)
	{
		accel.y += _controls.up * _acceleration.y;
	}
	else if (_controls.down)
	{
		accel.y -= _controls.down * _acceleration.y;
	}

	if (accel.x || accel.y)
	{
		accel *= 1.0f + _friction;
	}

	if (_controls.bJump && !airborne())
	{
		_jumping = true;
		impulse.z = _jumpAccel.y - level().gravity().z * delta;
		impulse.x += _jumpAccel.x * sin(rot().w);
	}

	if (_physics)
	{
		Point3 moveImpulse = impulse + accel * delta * scale;
		Point3 finalImpulse = physics()->impulse() + moveImpulse;

		if (physics()->vel().x + finalImpulse.x > _maxSpeed)
			moveImpulse.x = std::max(0.0f, _maxSpeed - physics()->vel().x - physics()->impulse().x);
		else if (physics()->vel().x + finalImpulse.x < -_maxSpeed)
			moveImpulse.x = std::min(0.0f, -_maxSpeed - physics()->vel().x - physics()->impulse().x);
		if (physics()->vel().y + finalImpulse.y > _maxSpeed)
			moveImpulse.y = std::max(0.0f, _maxSpeed - physics()->vel().y - physics()->impulse().y);
		else if (physics()->vel().y + finalImpulse.y < -_maxSpeed)
			moveImpulse.y = std::min(0.0f, -_maxSpeed - physics()->vel().y - physics()->impulse().y);

#if 0
		finalImpulse = physics()->impulse() + moveImpulse;
		if (physics()->vel().x + finalImpulse.x > _maxSpeed)
			int i = 0;
		else if (physics()->vel().x + finalImpulse.x < -_maxSpeed)
			int i = 0;
		if (physics()->vel().y + finalImpulse.y > _maxSpeed)
			int i = 0;
		else if (physics()->vel().y + finalImpulse.y < -_maxSpeed)
			int i = 0;
#endif

		physics()->applyImpulse(moveImpulse);
	}
}

float Man::normalisedAttackCharge() const
{
	WeaponMelee* w = Cast<WeaponMelee>(weapon());
	if (!w)
		return 0;

	if (*states != MeleeAttack)
		return 0;

	return clamp(((StateMeleeAttack*)(*states))->charge / w->chargeMaxTime());
}

void Man::concertina()
{
	states.go(Knockdown);
}

void Man::reset()
{
	states.go(Move);
	setPos(Point3(0, 0, 0));
	_physics->setPos(Point3(0, 0, physics()->radius()));
	_physics->setVel(Point3::ZERO);
}

void Man::makeHead(const Point3& localPos, const Point3& force, bool squashed)
{
	PtrGC<Head> newHead = new Head(
		localPos * worldXForm(),
		get("PancakeValue", 1.0f), 
		sprite().animator().set(), 
		squashed, 
		_headSquashedWeapon
	);

	newHead->setParent(level());

	if (newHead)
	{
		// Head may have been instantly destroyed if it intersected the walls.
		newHead->physics().setMass(get<float>("MassHead"));
		newHead->physics().applyForce(force);
	}
}

void Man::giveHealth(float amt)
{
	if (dead())
		return;

	_health = clamp(_health + amt, 0.0f, maxHealth());
}

PtrGC<class PhysSphere> Man::physics() const 
{ 
	return Cast<PhysSphere>(_physics); 
}

Point3 Man::projectileLocalLaunchPoint() const
{
	SpriteFXPos* o = Cast<SpriteFXPos>(sprite().animator()[0].spriteFX().findFirst(registrant("SpriteFXShootProjectile")->rtti, "attackranged"));
	if (o)
	{
		return Point3(o->pos().x, 0, o->pos().y);
	}

	return Point3::ZERO;
}

bool Man::disabled() const 
{ 
	return knockedDown()
		|| dead()
		|| concertinaed();
}

bool Man::concertinaPossible() const
{
	return _concertinaPossible;
}

bool Man::isAbove(const PhysicsObject& o) const
{
	float hitZDist = abs(physics()->pos().z - o.pos().z);

	return hitZDist > size().z * 0.5f;
}

void Man::decapitate(const Point3& headForce)
{
	if (!_bHead)
		return;

	_bHead = false;
	sprite().animator().play(Weapon::damageAnimationName(sprite().animator(), "hithead", _lastHitWeapon));
	makeHead(Point3(0, 0, size().z * 0.5f), headForce, false);
}

Point3 Man::modifiedKnockbackForce(const Point3& force, const char* damageType) const
{
	return states->modifiedKnockbackForce(force, damageType);
}

// STATES //////////////////////////////////////////////////////

Point3 Man::State::modifiedKnockbackForce(const Point3& force, const char* damageType) const
{
	// Allow 1.25 * knockdownTime seconds of opportunity to suffer enhanced knockback since knockdown.
	if (damageType != "rolling" && !o->_knockedDownAtTime.elapsed(o->level().time(), o->_knockdownTime * 1.25f))
		return force * o->_knockdownKnockbackFactor;
	else
		return force;
}

void Man::State::updateFacing()
{
	if (!o->_bDesiredLeft)
		o->setRot(QuatAngleAxis(PI, Point3(0, 0, 1)));
	else
		o->setRot(Quat(1, 0, 0, 0));
}

bool Man::shouldDecapitate(const char* damageType) const
{
	 return damageType == "maxcharge" && health() == 0;
}

void Man::State::onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType)
{
	o->_health = std::max(0.0f, o->_health - amt);

	bool shouldDecapitate = o->shouldDecapitate(damageType);
	bool applyForce = !shouldDecapitate;
	bool dead = o->health() == 0;

	if (applyForce)
		o->physics()->applyForce(o->modifiedKnockbackForce(knockback, damageType));

	if (shouldDecapitate)
	{
		o->decapitate(knockback);
	}
	else 
	{
		bool shouldKnockdown = (damageType == "jumpattack" && o->canBeKnockedDownByJumpAttack())
			|| damageType == "projectileknockdown";

		if (shouldKnockdown)
		{
			o->states.go(o->Knockdown);
		}
		else
		{
			if (dead)
			{
				o->sprite().animator().play(Weapon::damageAnimationName(o->sprite().animator(), "hitchest", o->_lastHitWeapon));
			}
			else
			{
				o->states.go(o->Hit);
				o->sprite().animator().play("knockback");
			}
		}
	}

	if (dead)
	{
		o->_killer = attacker;
		o->states.go(o->Dead);
	}
}

void Man::StateMove::start()
{
	bRollOnLand = false;
	o->_physics->setFriction(o->_friction);
}

void Man::StateMove::update(float delta)
{
	float scale;
	if (o->airborne())
		scale = o->_controlAir;
	else
		scale = 1.0f;

	o->processMovement(delta, scale);

	if (o->_jumping)
	{
		o->sprite().animator().play("jump", true);
	}
	else
	{
		if (!o->_controls.bJump)
		{
			if (o->groundSpeed() < 1.0f)
				o->sprite().animator().play("rest", true);
			else
			{
				o->sprite().animator().play("run", true);
				o->sprite().animator().setScale(Mapping::linear(clamp(o->groundSpeed(), 0.0f, o->_maxSpeed), 0.0f, o->_maxSpeed, 0.75f, 2.0f));
			}
		}
	}

	if (o->_controls.bFire)
	{
		if (o->canSquash())
			o->_squashTarget = o->findSquashTarget();
		else
			o->_squashTarget = 0;

		if (!o->_squashTarget)
			o->weapon()->primaryAttack();
		else
			o->states.go(o->Squash);
	}
}

void Man::StateMove::onCollide(class CollisionInfo& info)
{
	if (info.normal.z > 0.25f && bRollOnLand)
	{
		bRollOnLand = false;
		o->states.go(o->Roll);
	}
}

void Man::StateMove::onJumpedOnHead(Man& victim)
{
	o->physics()->setImpulse(o->physics()->impulse() - Point3(0, 0, o->physics()->vel().z) + o->_jumpedOnHeadImpulse);
	o->Roll->rollInstigator = &victim;
	bRollOnLand = true;
}

bool Man::StateMove::onHadHeadJumpedOn()
{
	if (!o->_bEnemy)
		return false;

	o->_concertinaAttemptTime = o->level().time().time();
	if (o->concertinaPossible())
	{
		o->concertina();
	}

	return true;
}

void Man::StateMove::end()
{
	o->sprite().animator().setScale(1.0f);
}

bool Man::StateAttack::onHadHeadJumpedOn()
{
	if (!o->_bEnemy)
		return false;

	o->_concertinaAttemptTime = o->level().time().time();
	if (o->concertinaPossible())
	{
		o->concertina();
	}

	return true;
}

void Man::StateMeleeAttack::start()
{
	if (!o->airborne())
	{
		o->sprite().animator().play("attackcharge", false, true);
		
		WeaponMelee* w = Cast<WeaponMelee>(o->weapon());
		// scale attack animation to charge time
		o->sprite().animator().setScale(o->sprite().animator().sequence().duration / w->chargeMaxTime());
		
		// kill motion
		o->physics()->setVel(Point3::ZERO);
	}
	else
	{
		o->sprite().animator().play("attackair", false, true);
	}

	bHit = false;
	swung = o->airborne();
	bMelee = false;
	charge = 0;

	airImpacted.clear();
}

void Man::StateMeleeAttack::end()
{
}

void Man::StateMeleeAttack::onMeleeHitRequest()
{
	bMelee = true;
}

void Man::StateMeleeAttack::update(float delta)
{
	Point3 playerDir = o->rot().vec(Point3(-1, 0, 0));

	o->updateFacing();
	
	if (o->airborne())
	{
		o->processMovement(delta, o->_controlAirAttack);
	}
	else if (!swung)
	{
		if (!o->_controls.bFire)
		{
			o->sprite().animator().setScale(1.0f);
			o->sprite().animator().play("attack");
			swung = true;
		}
		else
		{
			charge += delta;
		}
	}

	if (!bHit)
	{
		if (bMelee || (swung && o->sprite().animator().finished()))
		{
			WeaponMelee* w = Cast<WeaponMelee>(o->weapon());
			if (!o->airborne())
			{
				const char* damageType = "";

				if (charge >= w->chargeMaxTime())
					damageType = "maxcharge";

				// do damage
				w->setCharge(charge);
				w->damageArea(o->level(), o, w->attackOrigin(), w->attackExtent(), damageType);
			}

			bHit = true;
		}
	}
	
	if (bHit)
	{
		// do damage
		if (!o->airborne())
		{
			if (o->sprite().animator().finished())
				o->states.go(o->Move);
		}
		else
		{
			// damage in the air is calculated continuously
			std::vector<PtrGC<Physical> > hitResult;
			WeaponMelee* w = Cast<WeaponMelee>(o->weapon());
			w->queryArea(o->level(), o, w->attackOrigin(), w->airAttackExtent(), hitResult);
			for (uint i = 0; i < hitResult.size(); ++i)
			{
				if (airImpacted.insert(hitResult[i]).second == true)
				{
					w->damageObject(w->attackOrigin(), w->airAttackExtent(), hitResult[i], "jumpattack");
				}
			}
		}
	}
}

bool Man::canKnockDown() const
{
	return states->canKnockdown();
}

bool Man::canBeKnockedDownByJumpAttack() const
{
	return _knockdownPossibleJumpAttack && canKnockDown() && health() > 0;
}

bool Man::canBeKnockedDownByRoll() const
{
	return _knockdownPossibleRoll && _concertinaAttemptTime.elapsed(level().time(), 0.1f) && canKnockDown() && health() > 0;
}

////

void Man::StateRangeAttack::start()
{
	o->sprite().animator().play("attackranged");
	launched = false;
	launchVec = Point3::ZERO;
}

Point3 Man::StateRangeAttack::projectileDir() const
{
	if (o->_controls.projectileDirOverride != Point3::ZERO)
		return o->_controls.projectileDirOverride;

	Point3 dir;
	dir.x = -1 * o->_controls.left + 1 * o->_controls.right;
	dir.y = 1 * o->_controls.up + -1 * o->_controls.down;
	dir.z = 0;

	return dir;
}

void Man::StateRangeAttack::update(float delta)
{
	launchVec = projectileDir();
	launchVec.normalise();

	// safeguard if no launch anim
	if (o->sprite().animator().finished())
	{
		o->states.go(o->Move);
		if (!launched)
			onProjectileLaunchRequest(o->worldPos() + Point3(0, 0, o->size().z));
	}
}

void Man::StateRangeAttack::onProjectileLaunchRequest(const Point3& worldPos)
{
	launchVec = projectileDir();
	launchVec.normalise();

	// safeguard -- this can happen if shoot request happens on animation play
	if (launchVec == Point3::ZERO)
	{
		launchVec.x = -1 * o->_controls.left + 1 * o->_controls.right;
		launchVec.y = 1 * o->_controls.up + -1 * o->_controls.down;
		launchVec.z = 0;
	}

	WeaponRanged* w = Cast<WeaponRanged>(o->weapon());
	w->launchProjectile(worldPos, launchVec);
	launched = true;
}

void Man::StateRoll::start()
{
	o->sprite().animator().play("roll", true);
	rollTime = o->get<float>("RollTime");
	
	if (rollInstigator)
		 rollTime *= rollInstigator->get("RollTimeConcertinaFactor", 1.0f);

	o->_physics->setFriction(o->_rollFriction);
}

void Man::StateRoll::end()
{
	rollInstigator = 0;
	o->_physics->setFriction(o->_friction);
}

void Man::StateRoll::update(float delta)
{
	o->sprite().animator().setScale(Mapping::linear(o->groundSpeed(), 0.0f, o->_maxSpeed, 1.0f, 2.0f));

	rollTime -= delta;
	if (rollTime < 0)
	{
		o->states.go(o->Move);
	}

	o->processMovement(delta, o->_controlRoll);
}

void Man::StateRoll::onCollide(class CollisionInfo& info)
{
	PtrGC<Man> victim = Cast<Man>(info.victim->associatedObj());
	if (victim && victim->canBeKnockedDownByRoll())
	{
		victim->onDamage(o, 0, QuatFromVector(Point3(-1, 0, 0), Point3(o->physics()->vel().x, o->physics()->vel().y, 0).normal()) * o->_rollKnockback, AABB(), "", "rolling");
		victim->states.go(victim->Knockdown);
	}
}

void Man::StateRoll::onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType)
{
}

void Man::StateHit::start()
{
	std::string weapClass = strToLower(o->_lastHitWeapon);
	knockback = "knockback" + weapClass;
	counters.set(recover, 0.5f);
}

void Man::StateHit::update(float delta)
{
	State::update(delta);

	if (o->_health == 0)
	{
		if (o->airborne())
		{
			o->sprite().animator().play(knockback);
		}
		else
		{
			o->states.go(o->Dead);
		}
	}
	else
	{
		if (recover && !o->airborne())
			o->states.go(o->Move);
	}
}

void Man::StateDead::start()
{
	o->_bRespectBoundary = true;
	bWaitingToLand = o->airborne();
	o->_bPlayedDeath = false;

	o->setAIIndicator("");

	o->level().collisionGroup().remove(o->physics());
	o->level().deadGroup().add(o->physics());

	counters.set(10.0f, delegate(&StateDead::atRest));
}

void Man::StateDead::atRest()
{
	o->level().physicsMgr().remove(o->_physics);
}

void Man::StateDead::update(float delta)
{
	State::update(delta);

	if (!o->_bPlayedDeath)
	{
		if (((!bWaitingToLand || !o->airborne()) && o->sprite().animator().finished()))
		{
			std::string deathSeq = o->sprite().animator().sequence().name + std::string("dead");
			if (!o->sprite().animator().has(deathSeq))
				o->sprite().animator().play("dead");
			else
				o->sprite().animator().play(deathSeq);
			o->_bPlayedDeath = true;
		}
	}
}

void Man::StateSquash::start()
{
	bHit = false;
	o->sprite().animator().play("squashstart");
}

void Man::StateSquash::update(float delta)
{
	o->updateFacing();

	if (!bHit)
	{
		if (o->sprite().animator().finished())
		{
			Point3 origin = o->attackOrigin();
			origin.z = 0;

			bHit = true;
			o->sprite().animator().play("squashend");

			if (o->_squashTarget)
			{
				o->_squashTarget->onSquash(*o->_weapon);
			}
		}
	}
	else
	{
		if (o->sprite().animator().finished())
			o->states.go(o->Move);
	}
}

void Man::StateSquash::updateFacing()
{
	if (o->_squashTarget)
	{
		if (o->_squashTarget->worldPos().x >= o->worldPos().x)
			o->setRot(QuatAngleAxis(PI, Point3(0, 0, 1)));
		else
			o->setRot(Quat(1, 0, 0, 0));
	}
}

////

void Man::StateKnockdown::start()
{
	o->level().deadGroup().add(o->physics());
	o->level().collisionGroup().remove(o->physics());
	o->sprite().animator().clear();
	o->sprite().animator().play("knockdown");
	counters.set(o->_knockdownTime, delegate(&StateKnockdown::getup));
	o->_knockedDownAtTime = o->level().time();
}

void Man::StateKnockdown::getup()
{
	o->sprite().animator().play("getup");
	counters.set(o->sprite().animator().duration(), delegate(&StateKnockdown::finishGetup));
}

void Man::StateKnockdown::finishGetup()
{
	o->states.go(o->Move);
}

void Man::StateKnockdown::end()
{
	o->level().collisionGroup().add(o->physics());
	o->level().deadGroup().remove(o->physics());
}

Point3 Man::StateKnockdown::modifiedKnockbackForce(const Point3& force, const char* damageType) const
{
	return force * o->_knockdownKnockbackFactor;
}

// PlayerMan ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PlayerMan::PlayerMan(const std::string& characterType) :
	Man(characterType),
	_pancakes(0)
{
}

bool PlayerMan::ignoresHits() const 
{ 
	return level().options.get("Debug", "IgnoreEnemyHits", false);
}

void PlayerMan::onGotPancake(const PtrGC<class Head>& head)
{
	++_pancakes;

	App().game().onGotPancake(this, head);
}

// BaddyMan ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BaddyMan::BaddyMan(const std::string& characterType) :
	Man(characterType)
{
	_bRespectBoundary = false;
	_bEnemy = true;
}

void BaddyMan::constructObject(Object& parent)
{
	Super::constructObject(parent);

	if (App().options().get<bool>("Pancake Bros", "EnableAI", true))
	{
		AI* p = new AI(get<std::string>("AI", "AIBaddy"), get<std::string>("AISubtype", ""));
		p->setParent(parent.level());
		p->possess(this);
	}
}
