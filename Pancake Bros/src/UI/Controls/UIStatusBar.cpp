// ------------------------------------------------------------------------------------------------
//
// UIStatusBar
//
// ------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "UIStatusBar.h"
#include "RSE/AppBase.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/Materials/Material.h"
#include "Standard/Mapping.h"

IMPLEMENT_RTTI(UIStatusBar)

// style
UISTYLESTREAM(UIStatusBar)
	STREAMVAR(borderMaterialName);
	STREAMVAR(fillMaterialName);
	STREAMVAR(borderWidth);
UISTYLESTREAMEND

template<> void UIStatusBar::Style::onPostRead()
{
	borderMaterial = &material(borderMaterialName);
	fillMaterial = &material(fillMaterialName);

	if (!borderMaterial)
		throwf("No borderMaterialName defined for style.");

	if (!fillMaterial)
		throwf("No fillMaterialName defined for style.");
}

UIStatusBar::UIStatusBar() :
	m_value(0),
	m_min(0),
	m_max(1)
{
}

void UIStatusBar::setMin(float f)
{
	m_min = f;
}

void UIStatusBar::setMax(float f)
{
	m_max = f;
}

void UIStatusBar::onDraw()
{
	Super::onDraw();

	D3DPaint().setFill(*style().borderMaterial);
	D3DPaint().quad2D(screenPos().x, screenPos().y, screenPos().x + size().x, screenPos().x + size().y);
	D3DPaint().draw();

	if (m_value)
	{
		D3DPaint().setFill(*style().fillMaterial);
		float progress = Mapping::linear(*m_value, m_min, m_max, 0.0f, size().x - style().borderWidth * 2.0f);
		D3DPaint().uv0 = Point2(0, 0);
		D3DPaint().uv1 = Point2(Mapping::linear(*m_value, m_min, m_max, 0.0f, 1.0f), 1);
		D3DPaint().quad2D(
			screenPos().x + style().borderWidth, 
			screenPos().y + style().borderWidth, 
			screenPos().x + style().borderWidth + progress, 
			screenPos().y + size().y - style().borderWidth
		);
		D3DPaint().draw();
	}
}
