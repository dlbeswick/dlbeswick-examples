// ------------------------------------------------------------------------------------------------
//
// Hammer
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Weapon.h"


class Hammer : public WeaponMelee
{
	USE_RTTI(Hammer, WeaponMelee);
public:
	virtual float damageAmt();
protected:
private:
};