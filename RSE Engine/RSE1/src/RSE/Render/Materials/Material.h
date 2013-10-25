// ------------------------------------------------------------------------------------------------
//
// Material
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/Base.h>

class RSE_API Material : public Base
{
	USE_RTTI(Material, Base);
public:
	virtual void apply() const = 0;
};