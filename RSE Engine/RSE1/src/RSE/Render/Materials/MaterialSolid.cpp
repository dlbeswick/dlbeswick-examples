// ------------------------------------------------------------------------------------------------
//
// MaterialSolid
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "MaterialSolid.h"
#include "Render/SDeviceD3D.h"

REGISTER_RTTI(MaterialSolid)

MaterialSolid::MaterialSolid(
	const RGBA& _diffuse,
	const RGBA& _specular,
	const RGBA& _ambient,
	const RGBA& _emissive,
	float _power
	) :
	diffuse(_diffuse),
	specular(_specular),
	ambient(_ambient),
	emissive(_emissive),
	power(_power)
{
}

void MaterialSolid::streamVars(StreamVars& v)
{
	Super::streamVars(v);

	STREAMVAR(diffuse);
	STREAMVAR(specular);
	STREAMVAR(ambient);
	STREAMVAR(emissive);
	STREAMVAR(power);
}

void MaterialSolid::apply() const
{
	applySolidProperties();
	D3DD().SetTexture(0, 0);
}

void MaterialSolid::applySolidProperties() const
{
	D3DMATERIAL9 m;

	m.Diffuse = diffuse;
	m.Specular = specular;
	m.Ambient = ambient;
	m.Emissive = emissive;
	m.Power = power;
	
	D3DD().SetMaterial(&m);
	D3DD().SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	D3DD().SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	D3DD().SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	D3DD().SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	D3DD().SetRenderState(D3DRS_COLORVERTEX, false);
}