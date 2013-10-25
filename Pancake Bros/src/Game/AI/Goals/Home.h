#pragma once
#include "Selector.h"
#include "TargetBased.h"

namespace Goals
{
	class Home : public TargetBased
	{
		USE_RTTI(Home, TargetBased);
	public:
		Man::ControlBlock c;

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			parent->reselectGoal();

			// choose target
			assert(curTarget);

			zero(c);

			Point3 toTarget = curTarget->worldPos() - ai->obj()->worldPos();

			Man::ControlBlock c;
			zero(c);
			move(toTarget.normal(), c);
			ai->obj()->input(c);
		}
	};
}