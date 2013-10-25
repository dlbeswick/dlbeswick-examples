#pragma once
#include "TargetBased.h"
#include "RSE/Game/SpriteLayered.h"
#include "RSE/Game/TextureAnimatorLayered.h"

namespace Goals
{
	class Taunt : public TargetBased
	{
		USE_RTTI(Taunt, TargetBased);
	public:
		float minDist;
		std::string anim;

		Taunt(const std::string& _anim, float _minDist)
		{
			anim = _anim;
			minDist = _minDist;
		}

		virtual float rate(const PtrGC<Physical>& target)
		{
			if (!ai->obj()->canTaunt())
				return 0;

			PtrGC<Man> m = Cast<Man>(target);
			if (m && m->enemy(ai->obj()))
			{
				float score = distToScore((m->worldPos() - ai->obj()->worldPos()).length(), minDist);
				
				// allow a small chance to taunt disabled enemies in range
				if (m->disabled())
					score += 0.001f;

				if (!score)
					return 0.0f;
				else
					return 1.0f - score;
			}
			return 0;
		}

		virtual void start()
		{
			Point3 deltaPos = curTarget->worldPos() - obj()->worldPos();
			obj()->setFacing(deltaPos.x < 0);

			obj()->sprite().animator()[1].play(anim);
		}

		virtual void stop()
		{
			obj()->sprite().animator()[1].play("");
		}

		virtual bool completed()
		{
			return obj()->sprite().animator()[1].finished();
		}
	};
}