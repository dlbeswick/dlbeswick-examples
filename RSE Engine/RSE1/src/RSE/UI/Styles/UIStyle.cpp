// ------------------------------------------------------------------------------------------------
//
// UIStyle
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIStyle.h"
#include "AppBase.h"
#include "Render/Materials/Material.h"

IMPLEMENT_RTTI(UIStyle)
IMPLEMENT_RTTI(UIStyleBasic)

void UIStyleBasic::streamVars(StreamVars& v)
{
	Super::streamVars(v);

	STREAMVAR(borderMaterialName);
	STREAMVAR(borderWidth);
	STREAMVAR(fillMaterialName);
	STREAMVAR(font);
}

void UIStyleBasic::onPostRead()
{
	Super::onPostRead();

	borderMaterial = Cast<Material>(Base::streamFromConfig(AppBase().materials(), borderMaterialName));
	fillMaterial = Cast<Material>(Base::streamFromConfig(AppBase().materials(), fillMaterialName));

	if (!borderMaterial)
		throwf("No borderMaterialName defined for style.");

	if (!fillMaterial)
		throwf("No fillMaterialName defined for style.");
}