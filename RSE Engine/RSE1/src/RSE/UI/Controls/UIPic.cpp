// ------------------------------------------------------------------------------------------------
//
// UIPic
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UIPic.h"
#include "Render/SDeviceD3D.h"
#include "Render/D3DPainter.h"
#include "Render/TextureAnimator.h"
#include "Render/TextureAnimationTypes.h"
#include "Render/Materials/MaterialTexture.h"
#include "Render/Materials/MaterialAnimation.h"

IMPLEMENT_RTTI(UIPic)

UIPic::UIPic()
{
	m_material = 0;
}

UIPic::UIPic(const Material& material)
{
	m_material = &material;
	
	// tbd: this is convenient for setting UIPic size, but is it possible to avoid the cast here somehow?
	if (Cast<MaterialAnimation>(&material))
	{
		MaterialAnimation& m = *Cast<MaterialAnimation>(&material);
		setSize(m.animator().frame().pixels() / Point2i(D3D().screenSize().x, D3D().screenSize().x));
	}
}

UIPic::UIPic(const MaterialTexture& material)
{
	m_material = &material;
	setSize(Point2(material.sourceDimensions()) / Point2i(D3D().screenSize().x, D3D().screenSize().x));
}

UIPic::UIPic(const MaterialAnimation& material)
{
	m_material = &material;
	setSize(material.animator().frame().pixels() / Point2i(D3D().screenSize().x, D3D().screenSize().x));
}

void UIPic::set(const Material& m)
{
	m_material = &m;
}


void UIPic::draw()
{
	Super::draw();

	if (!m_material)
		return;

	D3DPaint().reset();
	D3DPaint().setFill(*m_material);
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();
}
