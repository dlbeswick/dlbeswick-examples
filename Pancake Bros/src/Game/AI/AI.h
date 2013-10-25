
// ------------------------------------------------------------------------------------------------
//
// AIBaddy
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Game/Controller.h"

class AI : public Controller
{
	USE_RTTI(AI, Controller);
public:
	AI(const std::string& name, const std::string& subtype);
};

