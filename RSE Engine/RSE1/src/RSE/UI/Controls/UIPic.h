// ------------------------------------------------------------------------------------------------
//
// UIPic
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class Material;
class MaterialTexture;
class MaterialAnimation;

class RSE_API UIPic : public UIElement
{
	USE_RTTI(UIPic, UIElement);
public:
	UIPic();
	UIPic(const Material& m);
	UIPic(const MaterialTexture& m);
	UIPic(const MaterialAnimation& m);

	void set(const Material& m);

	virtual void draw();

protected:
	const Material* m_material;
};