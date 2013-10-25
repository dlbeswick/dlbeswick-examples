// ------------------------------------------------------------------------------------------------
//
// AIRanged
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "RSEApp.h"
#include "AIRanged.h"
#include "Goals/AvoidDisabled.h"
#include "Goals/BaddyHome.h"
#include "Goals/MoveFromOOB.h"
#include "Goals/WeaponAttack.h"
#include <Standard/Config.h>

REGISTER_RTTI(AIRanged);

AIRanged::AIRanged()
{
}

void AIRanged::activated(PancakeLevel& level)
{
	Super::activated(level);

	add(new Goals::BaddyHome);
}