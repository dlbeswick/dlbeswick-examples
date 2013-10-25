#pragma once
#include "TargetBased.h"
#include <Standard/Rand.h>

namespace Goals
{
	class Panic : public TargetBased
	{
		USE_RTTI(Panic, TargetBased);
	public:
		Panic()
		{
			counters.loop(Rand(0.35f, 0.6f), delegate(&Panic::evalMove));
		}

		virtual void start()
		{
			obj()->setAIIndicator("panic");
			counters.set(Rand(6, 10), delegate(&Panic::finish));
			evalMove();
		}

		virtual void stop()
		{
			obj()->setAIIndicator("");
		}

		void evalMove()
		{
			Point3 dir(sin(Rand(0, 2.0f * PI)), cos(Rand(0, 2.0f * PI)), 0);

			moveDir = dir.normal();
		}

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);
			move(moveDir);
		}

		Point3 moveDir;
	};
}
