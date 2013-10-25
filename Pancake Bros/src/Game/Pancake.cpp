// ------------------------------------------------------------------------------------------------
//
// Pancake
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Pancake.h"
#include "RSEApp.h"
#include "Head.h"
#include "Man.h"
#include <Standard/Config.h>

IMPLEMENT_RTTI(Pancake)

Pancake::Pancake(Head& source)
{
	value = source.pancakeValue;
}

std::string Pancake::giveTo(const PtrGC<class Man> target)
{
	if (target->health() != target->maxHealth())
	{
		float bonus = App().options().get("Pancake Bros", "PancakeHealthAmount", 5.0f) * value;
		target->giveHealth(bonus);
		return "+ HEALTH";
	}
	else
	{
		return "+ " + itostr(App().options().get("Pancake Bros", "PancakeScoreAmount", 1000)) + " POINTS";
	}
}
