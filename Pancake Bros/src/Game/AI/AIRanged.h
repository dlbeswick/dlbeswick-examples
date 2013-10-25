// ------------------------------------------------------------------------------------------------
//
// AIRanged
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Game/ManAIControl.h"


class AIRanged : public AIBaddy
{
	USE_RTTI(AIRanged, AIBaddy);
public:
	AIRanged();

protected:
	virtual void activated(PancakeLevel& level);
};