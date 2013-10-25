#pragma once
#include "TargetBased.h"
#include "Standard/Range.h"

namespace Goals
{
	class Flee : public TargetBased
	{
		USE_RTTI(Flee, TargetBased);
	public:
		Flee(const Range<float>& _duration) :
			duration(_duration)
		{}

		virtual void start()
		{
			curDuration = duration();
		}

		virtual void activeUpdate(float delta)
		{
			TargetBased::activeUpdate(delta);

			if (curTarget)
			{
				move((ai->obj()->worldPos() - curTarget->worldPos()).normal());
				curDuration -= delta;
			}
		}
		
		virtual bool completed()
		{
			return !curTarget || curDuration <= 0;
		}

		Range<float> duration;
		float curDuration;
	};
}