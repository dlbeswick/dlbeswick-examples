// ------------------------------------------------------------------------------------------------
//
// AIBaddy
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "AI.h"
#include "RSEApp.h"
#include "Game/Man.h"
#include "Game/ManAIControl.h"
#include <Standard/Log.h>

IMPLEMENT_RTTI(AI);

AI::AI(const std::string& name, const std::string& subtype)
{
	setConfig(App().configAI());

    ManAIControl* newAI;

	newAI = Cast<ManAIControl>(Base::newObject(name));
	if (!newAI)
	{
		derr << "Unknown AI type " << name << ", destroying character.\n";
		self().destroy();
		return;
	}

	newAI->setConfigCategory(subtype);

	addMethod(Man::static_rtti(), newAI);
}