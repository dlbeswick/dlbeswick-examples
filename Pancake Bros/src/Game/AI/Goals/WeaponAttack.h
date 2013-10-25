#pragma once
#include "TargetBased.h"
#include "Game/AI/AIRanged.h"
#include "Game/Weapon.h"
#include "RSE/Physics/PhysSphere.h"
#include "Standard/Interpolation.h"
#include "Standard/Range.h"

namespace Goals
{
	class WeaponAttack : public TargetBased
	{
		USE_RTTI(WeaponAttack, TargetBased);
	public:
		WeaponAttack(const Range<float>& freq = Range<float>(0, 0)) :
			bAttacked(false),
			m_freq(freq)
		{
			counters.set(m_freqCounter, 1.0f / m_freq());
			m_freqCounter.trigger();
		}

		virtual void start()
		{
			Super::start();
			counters.set(m_freqCounter, 1.0f / m_freq());
		}

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			if (!curTarget)
			{
				finish();
				return;
			}

			if (!bAttacked)
			{
				Point3 toTarget = curTarget->worldPos() - ai->obj()->worldPos();

				Man::ControlBlock c;
				zero(c);
				calculateInput(c);
				ai->obj()->input(c);
				
				bAttacked = true;
			}
			else
			{
				if (!ai->obj()->attacking())
				{
					finish();
					bAttacked = false;
				}
				else
				{
					Man::ControlBlock c;
					zero(c);
					calculateInput(c);
					ai->obj()->input(c);
				}
			}
		}

		bool targetTooHigh(const PtrGC<Physical>& target) const
		{
			Point3 launchPoint = ai->obj()->projectileLocalLaunchPoint() * ai->obj()->worldXForm();
			return launchPoint.z > target->worldPos().z + target->physics().sphereBound();
		}

	protected:
		bool bAttacked;

		virtual void calculateInput(Man::ControlBlock& c)
		{
			if (bAttacked)
				return;

			Point3 toTarget = curTarget->worldPos() - ai->obj()->worldPos();
			move(toTarget.normal(), c);
			c.bFire = true;
		}

		virtual float suitability()
		{
			if (!m_freqCounter)
				return 0;
			else
				return Super::suitability();
		}

		bool isInRange(const PtrGC<Physical>& target)
		{
			return ai->obj()->weapon()->isInRange(target);
		}

		virtual float rate(const PtrGC<Physical>& target)
		{
			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->enemy(ai->obj()) && !m->disabled())
			{
				if (isInRange(target))
					return distToScore((m->worldPos() - ai->obj()->worldPos()).length());
			}
			return 0;
		}

		Range<float> m_freq;
		Counter m_freqCounter;
	};

	class WeaponAttackRanged : public WeaponAttack
	{
		USE_RTTI(WeaponAttackRanged, WeaponAttack);
	public:
		WeaponAttackRanged(const Range<float>& freq = Range<float>(0, 0), float leadSkill = 1.0f) :
			Super(freq),
			m_leadSkill(leadSkill)
		{
		}

	protected:
		Point3 futureTargetPoint() const
		{
			if (!curTarget)
				return Point3::ZERO;

			float weaponSpeed = 100.0f;
			
			WeaponRanged* w = Cast<WeaponRanged>(ai->obj()->weapon());
			if (w)
				weaponSpeed = w->projectileSpeed;

			Point3 toTarget = curTarget->worldPos() - (ai->obj()->projectileLocalLaunchPoint() * ai->obj()->worldXForm());
			float distToTarget = toTarget.length();
			float timeToTarget = distToTarget / weaponSpeed;
			
			return curTarget->worldPos() + curTarget->physics().vel() * timeToTarget;
		}

		virtual void calculateInput(Man::ControlBlock& c)
		{
			// aim at future destination of target based on ai lead skill
			Point3 targetPoint = Interpolation::linear(curTarget->worldPos(), futureTargetPoint(), m_leadSkill);
			
			Point3 toTarget = targetPoint - (ai->obj()->projectileLocalLaunchPoint() * ai->obj()->worldXForm());

			// if straight trajectory is too high for target, aim directly at target
			if (targetTooHigh(curTarget))
			{
				c.projectileDirOverride = toTarget.normal();
			}
			else
			{
				move(toTarget.normal(), c);
			}

			c.bFire = true;
		}
		
		float m_leadSkill;
	};

	class WeaponAttackRangedOnCharge : public WeaponAttackRanged
	{
		USE_RTTI(WeaponAttackRangedOnCharge, WeaponAttackRanged);
	public:
		WeaponAttackRangedOnCharge(const Range<float>& freq, float chargeAmt, float leadSkill = 1.0f) :
			Super(freq, leadSkill),
			_chargeAmt(chargeAmt)
		{
		}

	protected:
		float _chargeAmt;

		virtual float rate(const PtrGC<Physical>& target)
		{
			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->normalisedAttackCharge() > _chargeAmt)
				return Super::rate(target);
			else
				return 0;
		}
	};
}