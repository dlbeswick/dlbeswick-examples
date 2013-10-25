#pragma once
#include "Selector.h"
#include "TargetBased.h"
#include <Standard/Rand.h>

namespace Goals
{
	class BaddyHome : public TargetBased
	{
		USE_RTTI(BaddyHome, TargetBased);
	public:
		Counter cEvalMove;
		Man::ControlBlock c;

		BaddyHome()
		{
			zero(c);
		}

		virtual void activeUpdate(float delta)
		{
			Super::activeUpdate(delta);

			ai->obj()->input(c);
		}

		virtual void start()
		{
			Super::start();
			counters.loop(cEvalMove, Rand(0.35f, 0.6f), delegate(&BaddyHome::evalMove));
		}

		virtual void stop()
		{
			counters.set(cEvalMove, FLT_MAX);
		}

		void evalMove()
		{
			PtrGC<Man> o = Cast<Man>(ai->obj());

			if (!curTarget)
				return;

			// run toward target, trying to align with player 
			move((curTarget->worldPos() - o->worldPos()).normal(), c);

			parent->reselectGoal();

			// find intersection of us (large x and z box) and target
			/*AABB me(o->pos(), 
				Point3(
					(o->level().boundMax().x - o->level().boundMin().x) * 2, 
					o->extent().y,
					(o->level().boundMax().z - o->level().boundMin().z) * 2
					)
				);
			AABB him(curTarget->pos(), curTarget->extent());

			float yMapping;
			if (Collide::boxBox(me, him))
			{
				AABB intersection = Collide::boxBoxIntersection(me, him);

				// larger y intersection, go faster
				yMapping = Mapping::exp(intersection.extent.y, 0.0f, std::min(me.extent.y, him.extent.y), 0.0f, 1.0f);
			}
			else
				yMapping = 0;

			// go slower left and right if we're not lined up with the player
			c.left *= yMapping;
			c.right *= yMapping;

			// go slower up and down if we are lined up with the player
			c.up *= 1.0f - yMapping;
			c.down *= 1.0f - yMapping;*/
		}
	};
}
