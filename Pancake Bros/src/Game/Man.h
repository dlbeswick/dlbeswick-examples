// ------------------------------------------------------------------------------------------------
//
// Man
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Physical.h"
#include "Controller.h"
#include <Standard/StateMachine.h>
#include <Standard/Time.h>

class Weapon;


class Man : public Physical, public IControllable
{
	USE_RTTI(Man, Physical);
public:
	Man(const std::string& characterType);
	~Man();

	virtual void configLoad();

	struct ControlBlock
	{
		float left;
		float right;
		float up;
		float down;
		bool bJump;
		bool bFire;
		bool bSecondaryFire;
		Point3 projectileDirOverride;
	};

	void input(const ControlBlock& c);
	virtual void update(float delta);
	virtual void draw();

	virtual void reset();

	virtual void onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType);
	virtual void onSquash(Weapon& w);
	virtual void onMeleeHitRequest();
	virtual void onProjectileLaunchRequest(const Point2& pos);
	virtual void onHeadSquashRequest(const Point2& pos);

	virtual void meleeAttack() { states.go(MeleeAttack); }
	virtual void rangedAttack() { states.go(RangeAttack); }

	virtual void setFacing(bool bLeft);

	virtual void setAIIndicator(const std::string& name);

	// info
	virtual bool airborne() const;
	virtual bool attacking() const;
	virtual bool canBeSquashed() const { return dead() && !airborne() && !headless(); }
	virtual bool canKnockDown() const;
	virtual bool canBeKnockedDownByJumpAttack() const;
	virtual bool canBeKnockedDownByRoll() const;
	virtual bool canPickup() const { return !_bEnemy; }
	virtual bool canSquash() const { return !airborne(); }
	virtual bool canTaunt() const { return states->canTaunt(); }
	virtual bool concertinaed() const { return *states == Knockdown; }
	virtual bool concertinaPossible() const;
	virtual bool crushingFeet() const { return _crushingFeet; }
	virtual bool dead() const { return _health <= 0; }
	virtual void decapitate(const Point3& headForce);
	virtual bool didCrush(Man& victim) const;
	virtual bool disabled() const;
	virtual bool dying() const { return _health <= 0 && !_bPlayedDeath; }
	virtual bool enemy(const PtrGC<Man>& m) const { return m && _bEnemy != m->_bEnemy; }
	virtual Point3 extent() const { return _size; }
	virtual bool facingLeft() const { return rot() == Quat(1, 0, 0, 0); }
	virtual bool headless() const { return !_bHead; }
	virtual bool ignoresHits() const;
	virtual bool isAbove(const PhysicsObject& o) const;
	virtual Point2 jumpAccel() const { return _jumpAccel; }
	virtual PtrGC<Physical> killer() const { return _killer; }
	virtual bool knockedDown() const { return *states == Knockdown; }
	virtual float maxSpeed() const { return _maxSpeed; }
	virtual float normalisedAttackCharge() const;
	virtual Point3 projectileLocalLaunchPoint() const;
	virtual bool running() const;
	Point3 size() const { return _size; }
	
	virtual void onGotPancake(const PtrGC<class Head>& head) {}

	// accessor
	virtual const float& refHealth() const { return _health; }
	virtual float maxHealth() const { return _maxHealth; }
	virtual void giveHealth(float amt);
	virtual Weapon* weapon() const { return _weapon; }
	virtual Point3 attackOrigin();
	virtual const Point3& squashExtent() const;
	virtual PtrGC<Physical> findSquashTarget();

	PtrGC<class PhysSphere> physics() const;

protected:
	virtual void constructObject(Object& parent);
	virtual void createPhysics();

	virtual void updateFacing();
	void onCollide(class CollisionInfo& info);
	virtual Point3 modifiedKnockbackForce(const Point3& force, const char* damageType) const;
	virtual void processMovement(float delta, float scale);
	virtual bool didJumpOnHead(const CollisionInfo& info) const;
	virtual void concertina();
	virtual bool shouldDecapitate(const char* damageType) const;

	virtual void makeHead(const Point3& localPos, const Point3& force, bool squashed);

	// states

	EXTEND_STATECLASS(Man)
	{
		virtual bool canKnockdown() const { return true; }
		virtual Point3 modifiedKnockbackForce(const Point3& force, const char* damageType) const;
		virtual void onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType);
		virtual void onProjectileLaunchRequest(const Point3& worldPos) {}
		virtual void onMeleeHitRequest() {}
		virtual void onJumpedOnHead(Man& victim) {}
		virtual void onCollide(class CollisionInfo& info) {};
		virtual bool onHadHeadJumpedOn() { return false; }
		virtual bool canTaunt() const { return false; }
		virtual bool isAttacking() const { return false; }
		virtual void updateFacing();
	};

	DECLARE_STATEMACHINE

	STATE(Move)
	{
		void start();
		void end();
		void update(float delta);
		virtual void onJumpedOnHead(Man& victim);
		virtual bool onHadHeadJumpedOn();
		virtual bool canTaunt() const { return !o->airborne(); }
		void onCollide(class CollisionInfo& info);

		bool bRollOnLand;
	};
	
	STATE(Attack)
	{
		virtual bool isAttacking() const { return true; }
		virtual bool onHadHeadJumpedOn();
	};
	
	EXTEND_STATE(MeleeAttack, StateAttack)
	{
		void start();
		void update(float delta);
		void end();

		void onMeleeHitRequest();

		bool bHit;
		bool bMelee;
		bool swung;
		float charge;
		std::set<PtrGC<Physical> > airImpacted;
	};
	
	EXTEND_STATE(RangeAttack, StateAttack)
	{
		void start();
		void update(float delta);
		void onProjectileLaunchRequest(const Point3& worldPos);
		Point3 projectileDir() const;

		Point3 launchVec;
		bool launched;
	};
	
	STATE(Squash)
	{
		void start();
		void update(float delta);
		void updateFacing();

		bool bHit;
	};

	STATE(Roll)
	{
		virtual bool canKnockdown() const { return false; }
		void start();
		void end();
		void update(float delta);
		void onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType);
		void onCollide(class CollisionInfo& info);

		PtrGC<Man> rollInstigator;
		float rollTime;
	};
	
	STATE(Hit)
	{
		void start();
		void update(float delta);

		std::string knockback;
		std::string dead;
		Counter recover;
	};
	
	STATE(Dead)
	{
		void start();
		void update(float delta);
		virtual void onDamage(const PtrGC<Man>& attacker, float amt, const Point3& knockback, const AABB& localHit, const std::string& weaponString, const char* damageType) {};

		void atRest();

		bool bWaitingToLand;
	};
	
	STATE(Knockdown)
	{
		void start();
		void end();
		void getup();
		void finishGetup();

		virtual Point3 modifiedKnockbackForce(const Point3& force, const char* damageType) const;
	};

	ControlBlock _controls;

	Point3 _boundMin;
	Point3 _boundMax;
	bool _bLeft;
	bool _bDesiredLeft;
	bool _concertinaPossible;
	Time _concertinaAttemptTime;
	float _controlAir;
	float _controlAirAttack;
	float _controlRoll;
	float _fakeShadowAdjustment;
	bool _jumping;
	float _knockdownKnockbackFactor;
	bool _knockdownPossibleJumpAttack;
	bool _knockdownPossibleRoll;
	Time _knockedDownAtTime;
	float _maxSpeed;
	float _friction;
	float _rollFriction;
	Point3 _rollKnockback;
	float _rollSpeedFraction;
	Point3 _acceleration;
	Point2 _jumpAccel;
	Point3 _jumpedOnHeadImpulse;
	Point3 _headAttackAccel;
	Point3 _squashExtent;
	float _headArea;
	float _chestArea;
	float _legArea;
	float _maxHealth;
	float _knockdownTime;
	float _squashCautionRadius;
	std::string _lastHitWeapon;
	Weapon* _headSquashedWeapon;
	Weapon* _weapon;
	AABB _lastHit;
	PtrGC<Physical> _squashTarget;

	bool _bEnemy;
	bool _bHead;
	bool _bPlayedDeath;
	bool _lastWasLeft;
	bool _crushingFeet;
	PtrGC<Physical> _killer;

	PtrGC<class Sprite> _aiIndicator;
};

class PlayerMan : public Man
{
	USE_RTTI(PlayerMan, Man);
public:
	PlayerMan(const std::string& characterType);

	virtual bool ignoresHits() const;
	virtual void onGotPancake(const PtrGC<class Head>& head);
	virtual int pancakes() const { return _pancakes; }

protected:
	int _pancakes;
};

class BaddyMan : public Man
{
	USE_RTTI(BaddyMan, Man);

public:
	BaddyMan(const std::string& characterType);

	virtual bool canSquash() const { return false; }

protected:
	virtual void constructObject(Object& parent);
};