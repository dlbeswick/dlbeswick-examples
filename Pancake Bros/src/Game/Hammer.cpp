// ------------------------------------------------------------------------------------------------
//
// Hammer
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "RSEApp.h"
#include "Hammer.h"
#include "Man.h"

REGISTER_RTTI(Hammer);

float Hammer::damageAmt()
{
	float d = Super::damageAmt();

	if (m_man->airborne())
		d *= 0.5f;

	return d;
}