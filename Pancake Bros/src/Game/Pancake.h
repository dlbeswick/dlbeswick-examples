// ------------------------------------------------------------------------------------------------
//
// Pancake
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/Base.h"
#include "Standard/PtrGC.h"

class Pancake : public Base
{
	USE_RTTI(Pancake, Base);
public:
	Pancake(class Head& source);

	std::string giveTo(const PtrGC<class Man> target);

protected:
	float value;
};