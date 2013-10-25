#include "stdafx.h"

#include "AvoidDisabled.h"
#include "BaddyHome.h"
#include "Flee.h"
#include "Home.h"
#include "JumpAttack.h"
#include "MoveFromOOB.h"
#include "Panic.h"
#include "PanicGore.h"
#include "Selector.h"
#include "Sequencer.h"
#include "TargetBased.h"
#include "Taunt.h"
#include "WeaponAttack.h"

namespace Goals
{
	IMPLEMENT_RTTI(AvoidDisabled);
	IMPLEMENT_RTTI(BaddyHome);
	IMPLEMENT_RTTI(WeaponAttack);
	IMPLEMENT_RTTI(WeaponAttackRanged);
	IMPLEMENT_RTTI(JumpAttack);
	IMPLEMENT_RTTI(Flee);
	IMPLEMENT_RTTI(Home);
	IMPLEMENT_RTTI(MoveFromOOB);
	IMPLEMENT_RTTI(Panic);
	IMPLEMENT_RTTI(PanicGore);
	IMPLEMENT_RTTI(TargetBased);
	IMPLEMENT_RTTI(Taunt);
	IMPLEMENT_RTTI(Sequencer);
	IMPLEMENT_RTTI(Selector);
	IMPLEMENT_RTTI(WeaponAttackRangedOnCharge);
}