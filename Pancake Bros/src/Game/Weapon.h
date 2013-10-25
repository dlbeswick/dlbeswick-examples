// ------------------------------------------------------------------------------------------------
//
// Weapon
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/Base.h"
#include "Standard/Lazy.h"
#include "Standard/PtrGC.h"
#include "Standard/Registrar.h"

class Man;
class Physical;


class Weapon : public Base, public EmbeddedPtrHost
{
	USE_RTTI(Weapon, Base);
public:
	Weapon();

	virtual void queryArea(class PancakeLevel& l, class Man* attacker, const Point3& pos, Point3 extent, std::vector<PtrGC<Physical> >& result);
	virtual void damageArea(class PancakeLevel& l, class Man* attacker, const Point3& pos, Point3 extent, const char* damageType = "");
	virtual void damageObject(const Point3& pos, Point3 extent, const PtrGC<Physical>& target, const char* damageType = "");
	virtual Point3 knockback();
	virtual float damageAmt();

	virtual const std::string& weaponID();

	virtual void playHitEffect(class PancakeLevel& l, const Point3& pos, const std::string& base = "hiteffect");
	static std::string damageAnimationName(class ITextureAnimator& source, const std::string& base, const std::string& weaponString);
	std::string damageAnimationName(class ITextureAnimator& source, const std::string& base);

	void setMan(Man* m);
	Man* man() const { return m_man; }

	virtual void primaryAttack() {};
	virtual void secondaryAttack() {};

	virtual bool isInRange(const PtrGC<Physical>& target) { return false; }
	virtual Point3 attackOrigin() { return Point3::ZERO; }

protected:
	Lazy<Point3> m_knockback;
	Lazy<float> m_damageAmt;
	Lazy<std::string> m_weaponID;
	Man* m_man;
};

class WeaponMelee : public Weapon
{
	USE_RTTI(WeaponMelee, Weapon);
public:
	virtual float damageAmt();
	virtual Point3 knockback();

	virtual Point3 attackExtent();
	virtual Point3 airAttackExtent();
	virtual float chargeKnockbackFactor();
	virtual float chargeMaxTime();
	virtual float chargeDamageFactor();
	virtual float chargeNormalised();

	virtual void primaryAttack();
	virtual void secondaryAttack();

	virtual Point3 attackOrigin();

	virtual bool isInRange(const PtrGC<Physical>& target);

	void setCharge(float charge) { m_charge = charge; }

protected:
	Lazy<Point3> m_attackExtent;
	Lazy<Point3> m_airAttackExtent;
	Lazy<float> m_chargeKnockbackFactor;
	Lazy<float> m_chargeMaxTime;
	Lazy<float> m_chargeDamageFactor;
	float m_charge;
};

class WeaponRanged : public Weapon
{
	USE_RTTI(WeaponRanged, Weapon);
public:
	virtual void construct();

	virtual Point3 attackOrigin() { return Point3::ZERO; }

	virtual bool isInRange(const PtrGC<Physical>& target);

	virtual void primaryAttack();
	virtual void launchProjectile(const Point3& worldPos, const Point3& dir);

	float projectileGravity;
	bool projectileKnockdown;
	float projectileSize;
	float projectileSpeed;

protected:
	friend class Projectile;

	float projectileRange;	// range in world screen units

	std::string m_projectileClass;
};