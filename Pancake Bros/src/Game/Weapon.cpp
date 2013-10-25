// ------------------------------------------------------------------------------------------------
//
// Weapon
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Weapon.h"
#include "RSEApp.h"
#include "PancakeLevel.h"
#include "Man.h"
#include "Projectile.h"
#include "RSE/Physics/PhysicsObject.h"
#include "RSE/Render/TextureAnimator.h"
#include "RSE/Render/TextureAnimationManager.h"
#include "RSE/Render/ParticleFactory.h"
#include "RSE/Render/ParticleSystem.h"
#include "Standard/Collide.h"
#include "Standard/Mapping.h"

REGISTER_RTTI(Weapon);

Weapon::Weapon()
{
	setConfig(App().configGame());
}

Point3 Weapon::knockback()
{
	if (!m_knockback.evaluated())
		m_knockback = get<Point3>("Knockback");
	return (Point3)m_knockback;
}

float Weapon::damageAmt()
{
	if (!m_damageAmt.evaluated())
		m_damageAmt = get<float>("DamageAmt");
	return (float)m_damageAmt;
}

void Weapon::damageArea(PancakeLevel& l, Man* attacker, const Point3& pos, Point3 extent, const char* damageType)
{
	std::vector<PtrGC<Physical> > result;
	queryArea(l, attacker, pos, extent, result);

	for (uint i = 0; i < result.size(); ++i)
	{
		damageObject(pos, extent, result[i], damageType);
	}
}

void Weapon::damageObject(const Point3& pos, Point3 extent, const PtrGC<Physical>& target, const char* damageType)
{
	Point3 kb = knockback();

	if (m_man)
		kb *= m_man->worldRot();

	AABB weaponBox(pos, extent);
	AABB targetBox(target->pos(), target->size());
	AABB hitBox = Collide::aabbAABBIntersection(weaponBox, targetBox);
	
	target->onDamage(m_man, damageAmt(), kb, hitBox, weaponID(), damageType);
	playHitEffect(target->level(), hitBox.origin);
}

void Weapon::playHitEffect(class PancakeLevel& l, const Point3& pos, const std::string& base)
{
	ParticleSystem* s = l.particles(l, strToLower(rtti().className()) + "_" + base);
	if (s)
		s->setPos(pos);
}

void Weapon::queryArea(class PancakeLevel& l, class Man* attacker, const Point3& pos, Point3 extent, std::vector<PtrGC<Physical> >& result)
{
	result.resize(0);

	const Level::ObjectList& c = l.autoList<Physical>();
	for (Level::ObjectList::const_iterator i = c.begin(); i != c.end(); ++i)
	{
		PtrGC<Physical> m = Cast<Physical>(*i);
		if (!m || m == attacker || m->health() <= 0)
			continue;

		AABB weaponBox(pos, extent);
		AABB targetBox(m->worldPos(), m->size());

		if (Collide::aabbAABB(weaponBox, targetBox))
		{
			result.push_back(m);
		}
	}

}

void Weapon::setMan(Man* m)
{
	m_man = m; 
}

const std::string& Weapon::weaponID()
{
	if (m_weaponID.evaluated())
		return m_weaponID;
	
	m_weaponID->resize(configCategory().size());
	std::transform(configCategory().begin(), configCategory().end(), m_weaponID->begin(), tolower);
	return m_weaponID;
}

std::string Weapon::damageAnimationName(class ITextureAnimator& source, const std::string& base, const std::string& weaponString)
{
	std::string anim = base + weaponString;
	if (source.has(anim))
		return anim;
	else
		return base;
}

std::string Weapon::damageAnimationName(class ITextureAnimator& source, const std::string& base)
{
	return damageAnimationName(source, base, weaponID());
}

///////////////////////////////////////////////////////

REGISTER_RTTI(WeaponMelee);

void WeaponMelee::primaryAttack()
{
	m_man->meleeAttack();
}

void WeaponMelee::secondaryAttack()
{
}

Point3 WeaponMelee::attackExtent()
{
	if (!m_attackExtent.evaluated())
		m_attackExtent = get<Point3>("AttackExtent");
	return (Point3)m_attackExtent;
}

Point3 WeaponMelee::airAttackExtent()
{
	if (!m_airAttackExtent.evaluated())
		m_airAttackExtent = get<Point3>("AirAttackExtent");
	return (Point3)m_airAttackExtent;
}

Point3 WeaponMelee::attackOrigin()
{
	Point3 playerDir = m_man->worldRot().vec(Point3(-1, 0, 0));
	if (m_man->airborne())
		return m_man->worldPos() + Point3(playerDir.x * airAttackExtent().x * 0.5f, 0, 0);
	else
		return m_man->worldPos() + Point3(playerDir.x * attackExtent().x * 0.5f, 0, 0);
}

bool WeaponMelee::isInRange(const PtrGC<Physical>& target)
{
	return Collide::aabbAABB(AABB(attackOrigin(), attackExtent()), AABB(target->worldPos(), target->extent()));
}

float WeaponMelee::chargeKnockbackFactor()
{
	if (!m_chargeKnockbackFactor.evaluated())
		m_chargeKnockbackFactor = get("ChargeKnockbackFactor", 1000.0f);
	return m_chargeKnockbackFactor;
}

float WeaponMelee::chargeMaxTime()
{
	if (!m_chargeMaxTime.evaluated())
		m_chargeMaxTime = get("ChargeMaxTime", 1.0f);
	return m_chargeMaxTime;
}

float WeaponMelee::chargeDamageFactor()
{
	if (!m_chargeDamageFactor.evaluated())
		m_chargeDamageFactor = get("ChargeDamageFactor", 100.0f);
	return m_chargeDamageFactor;
}

Point3 WeaponMelee::knockback()
{
	return Super::knockback() * Mapping::exp(chargeNormalised(), 0.0f, 1.0f, 1.0f, chargeKnockbackFactor());
}

float WeaponMelee::damageAmt()
{
	return Super::damageAmt() * Mapping::exp(chargeNormalised(), 0.0f, 1.0f, 1.0f, chargeDamageFactor());
}

float WeaponMelee::chargeNormalised()
{
	return Mapping::linear(clamp((const float&)m_charge, 0.0f, chargeMaxTime()), 0.0f, chargeMaxTime(), 0.0f, 1.0f);
}

///////////////////////////////////////////////////////

REGISTER_RTTI(WeaponRanged);

void WeaponRanged::construct()
{
	Super::construct();
	setConfig(App().configGame());
	m_projectileClass = get<std::string>("ProjectileClass");
	projectileGravity = get("ProjectileGravity", 0.0f);
	projectileRange = get("ProjectileRange", 0.0f);
	projectileSize = get("ProjectileSize", 0.0f);
	projectileSpeed = get("ProjectileSpeed", 0.0f);
	projectileKnockdown = get("ProjectileKnockdown", false);

	if (m_projectileClass.empty())
		dlog << "Projectile " << configCategory() << " has no ProjectileClass given.\n";
}

void WeaponRanged::primaryAttack()
{
	m_man->rangedAttack();
}

void WeaponRanged::launchProjectile(const Point3& worldPos, const Point3& dir)
{
	Projectile* p = (Projectile*)newObjectArg1(registrant(m_projectileClass)->rtti, this);
	p->setParent(man()->level());
	p->physics().setVel(dir * projectileSpeed);
	p->physics().setPos(worldPos);
}

bool WeaponRanged::isInRange(const PtrGC<Physical>& target)
{
	return (target->pos() - m_man->pos()).length() <= projectileRange * m_man->level().boundLength2D();
}
