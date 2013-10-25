#pragma once
#include "TargetBased.h"
#include "Game/Weapon.h"
#include "RSE/Physics/PhysSphere.h"
#include "RSE/Render/D3DPainter.h"
#include "Standard/Algebra.h"

namespace Goals
{
	class JumpAttack : public TargetBased
	{
		USE_RTTI(JumpAttack, TargetBased);
	public:
		JumpAttack(float freq = 0.5f) :
			bAttacked(false),
			m_chosenVel(Point3::ZERO),
			m_launchPoint(Point3::ZERO)
		{
			if (freq)
			{
				counters.set(m_freq, 1.0f / freq);
				m_freq.trigger();
			}
			else
			{
				counters.set(m_freq, 0.0f);
			}

			m_targetTime = 0.5f;
		}

		virtual void start()
		{
			Super::start();
			counters.set(m_freq);

			bAttacked = false;

			m_chosenVel = jumpVel(curTarget);
			m_launchPoint = ai->obj()->worldPos();
			Man::ControlBlock c;

			counters.set(m_waitForJump, 0.1f);

			zero(c);
			c.bJump = true;
			ai->obj()->input(c);
			ai->obj()->physics()->setVel(m_chosenVel);
		}

#if 0
		virtual void update(float delta)
		{
			Super::update(delta);
			{
				DeferredPainter p(ai->obj()->level());
				Point3 vel = jumpVel(curTarget);
				if (canJumpFor(vel))
					p->setFill(RGBA(0,1,0));
				else
					p->setFill(RGBA(1,0,0));
				p->projectile(ai->obj()->worldPos(), vel, ai->obj()->level().gravity());
			}
			{
				DeferredPainter p(ai->obj()->level());
				p->setFill(RGBA(0,1,1));
				p->projectile(m_launchPoint, m_chosenVel, ai->obj()->level().gravity());
			}
		}
#endif

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			if (!bAttacked && ai->obj()->physics()->vel().z < 0)
			{
				Man::ControlBlock c;
				zero(c);
				c.bFire = true;
				ai->obj()->input(c);
				bAttacked = true;
			}
			if (m_waitForJump && !ai->obj()->airborne())
			{
				finish();
			}
/*			else
			{
				if (!ai->obj()->attacking())
				{
					finish();
					bAttacked = false;
				}
				else
				{
					Point3 toTarget = curTarget->worldPos() - ai->obj()->worldPos();

					Man::ControlBlock c;
					zero(c);
					move(toTarget.normal(), c);
					ai->obj()->input(c);
				}
			}*/
		}

	protected:
		bool bAttacked;

		Point3 jumpVel(const PtrGC<Physical>& target)
		{
			if (!target)
				return Point3::ZERO;
			return Algebra::projectileVelocityGivenTime(ai->obj()->worldPos(), attackPos(target), ai->obj()->level().gravity().z, m_targetTime);
		}

		virtual float suitability()
		{
			if (!m_freq)
				return 0;
			else
				return Super::suitability();
		}

		bool canJumpFor(const Point3& vel)
		{
			return abs(vel.x) <= ai->obj()->jumpAccel().x;
		}

		bool isInRange(const PtrGC<Physical>& target)
		{
			return canJumpFor(jumpVel(target));
		}

		Point3 attackPos(const PtrGC<Physical>& target)
		{
			return target->physics().worldPos() + target->physics().vel() * m_targetTime;
		}

		virtual float rate(const PtrGC<Physical>& target)
		{
			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->enemy(ai->obj()) && !m->dead())
			{
				if (isInRange(target))
					return distToScore((m->worldPos() - ai->obj()->worldPos()).length());
			}
			return 0;
		}

		Counter m_freq;
		Counter m_waitForJump;
		Point3 m_chosenVel;
		Point3 m_launchPoint;
		float m_targetTime;
	};
}