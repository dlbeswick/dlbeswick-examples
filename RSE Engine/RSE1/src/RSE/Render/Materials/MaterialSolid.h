// ------------------------------------------------------------------------------------------------
//
// MaterialSolid
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Material.h"

class RSE_API MaterialSolid : public Material
{
	USE_RTTI(MaterialSolid, Material);
public:
	RGBA diffuse;
	RGBA specular;
	RGBA ambient;
	RGBA emissive;
	float power;

	MaterialSolid(
		const RGBA& _diffuse = RGBA(1,1,1),
		const RGBA& _specular = RGBA(1,1,1),
		const RGBA& _ambient = RGBA(0,0,0,0),
		const RGBA& _emissive = RGBA(0,0,0,0),
		float _power = 1
		);

	virtual void apply() const;

protected:
	virtual void applySolidProperties() const;
	virtual void streamVars(StreamVars& v);
};