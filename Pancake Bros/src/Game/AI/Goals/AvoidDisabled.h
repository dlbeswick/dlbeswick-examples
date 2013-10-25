#pragma once
#include "Flee.h"
#include "RSE/Physics/PhysSphere.h"

namespace Goals
{
	class AvoidDisabled : public Flee
	{
		USE_RTTI(AvoidDisabled, Flee);
	public:
		AvoidDisabled(float range=0.0f) :
			Super(0), m_range(range)
		{}

		virtual float range()
		{
			if (!m_range)
				return ai->obj()->physics()->sphereBound() * 2.0f;

			return m_range;
		}

		virtual bool completed()
		{
			return !curTarget || !isInRange(curTarget);
		}

		bool isInRange(const PtrGC<Physical>& target)
		{
			return ((target->worldPos() - ai->obj()->worldPos()).rComponentMul(Point3(1,1,0))).length() < range();
		}

		virtual float rate(const PtrGC<Physical>& target)
		{
			float score = 0;

			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->enemy(ai->obj()) && m->disabled())
			{
				if (isInRange(target))
					score = distToScore((m->worldPos() - ai->obj()->worldPos()).length());
			}

			return score;
		}

	protected:
		float m_range;
	};
}