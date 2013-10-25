// ------------------------------------------------------------------------------------------------
//
// UIStyle
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include <Standard/Base.h>

class Material;


class RSE_API UIStyle : public Base
{
	USE_RTTI(UIStyle, Base);
};

class RSE_API UIStyleBasic : public UIStyle
{
	USE_RTTI(UIStyleBasic, UIStyle);
public:
	float		borderWidth;
	Material*	borderMaterial;
	Material*	fillMaterial;
	std::string	font;
	
protected:
	std::string borderMaterialName;
	std::string fillMaterialName;

	virtual void streamVars(StreamVars& v);
	virtual void onPostRead();
};