#pragma once
#include "Game/PancakeLevel.h"
#include "Game/AI/AIGoal.h"
#include "Game/ManAIControl.h"

namespace Goals
{
	class TargetBased : public AIGoal
	{
		USE_RTTI(TargetBased, AIGoal);
	public:
		typedef stdext::hash_set<PtrGC<Physical> > Targets;
		PtrGC<Physical> curTarget;

		virtual float suitability()
		{
			chooseTarget();
			if (!curTarget)
				return 0;

			return rate(curTarget);
		}

		virtual bool completed()
		{
			return !curTarget;
		}

	protected:
		float distToScore(float dist, float minRange = 0.0f, float maxRange = 0.0f)
		{
			if (!maxRange)
				maxRange = ai->obj()->level().boundLength();
			
			if (dist > maxRange || dist < minRange)
				return 0;

			return 1.0f - ((dist - minRange) / (maxRange - minRange));
		}

		float distToScore2D(const Point3& p0, const Point3& p1, float minRange = 0.0f, float maxRange = 0.0f)
		{
			Point3 p = p1 - p0;
			p.z = 0;

			float dist = p.length();

			if (!maxRange)
				maxRange = ai->obj()->level().boundLength2D();
			
			if (dist > maxRange || dist < minRange)
				return 0;

			return 1.0f - ((dist - minRange) / (maxRange - minRange));
		}

		virtual float rate(const PtrGC<Physical>& target)
		{
			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->enemy(ai->obj()) && !m->disabled())
			{
				return distToScore((target->worldPos() - ai->obj()->worldPos()).length());
			}
			else
				return 0;
		}

	private:
		void chooseTarget()
		{
			Targets targets;

			const Level::ObjectList& c = ai->obj()->level().autoList<Physical>();
			for (Level::ObjectList::const_iterator i = c.begin(); i != c.end(); ++i)
			{
				PtrGC<Physical> o = Cast<Physical>(*i);
				if (o)
					targets.insert(o);
			}

			float maxScore = 0;
			PtrGC<Physical> bestTarget;
			if (!targets.empty())
			{
				float score;
				for (Targets::iterator i = targets.begin(); i != targets.end(); ++i)
				{
					score = rate(*i);
					if (score > maxScore)
					{
						maxScore = score;
						bestTarget = *i;
					}
				}

				curTarget = bestTarget;
			}
			else
				curTarget = 0;
		}
	};
}