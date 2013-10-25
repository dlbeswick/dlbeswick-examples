#pragma once
#include "Game/AI/AIGoal.h"

namespace Goals
{
	class MoveFromOOB : public AIGoal
	{
		USE_RTTI(MoveFromOOB, AIGoal);

		virtual float suitability()
		{
			if (!completed())
				return 1.0f;
			else
				return 0.0f;
		}
		virtual bool completed()
		{
			const Point3& boundMin = ai->obj()->level().boundMin();
			const Point3& boundMax = ai->obj()->level().boundMax();
			const Point3& pos = ai->obj()->worldPos();
			
			return pos.x - ai->obj()->size().x * 0.5f > boundMin.x && pos.x + ai->obj()->size().x * 0.5f < boundMax.x;
		}
		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			Man::ControlBlock c;
			zero(c);
			if (ai->obj()->worldPos().x < 0)
				move(Point3(1, 0, 0));
			else
				move(Point3(-1, 0, 0));
		};
	};
}