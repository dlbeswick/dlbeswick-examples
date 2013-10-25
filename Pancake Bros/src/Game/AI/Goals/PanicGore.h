#pragma once
#include "Panic.h"

namespace Goals
{
	class PanicGore : public Panic
	{
		USE_RTTI(PanicGore, Panic);
	public:
		virtual float rate(const PtrGC<Physical>& target)
		{
			PtrGC<Man> m = Cast<Man>(target);
			if (m && !m->enemy(ai->obj()) && m->dying() && m->headless())
				return distToScore((target->worldPos() - ai->obj()->worldPos()).length());
			else
				return 0;
		}
	};
}