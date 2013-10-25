// ------------------------------------------------------------------------------------------------
//
// TransitionFade
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "TransitionFade.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include "UI/DialogMgr.h"

IMPLEMENT_RTTI(TransitionFade);

TransitionFade::TransitionFade(const PtrGC<UIElement>& src, float time) :
	Super(src, time)
{
}

void TransitionFade::draw()
{
	Super::draw();

	// tbd: investigate why this is null (caused by immediate add then draw?)
	// it was because of me not adding a dialog to a branch before calling the transition
	if (!m_src->uiBranch())
		return;

	float shift = DialogMgr().camera().perPixelShift();

	D3D().zbuffer(false);
	D3DPaint().reset();
	D3DPaint().setFill(RGBA(1,1,1,alpha()), m_tex);
	D3DPaint().uv0 = Point2::ZERO;
	D3DPaint().uv1 = Point2(1,1);
	D3DPaint().quad2D(shift, shift, m_src->uiBranch()->size().x + shift, m_src->uiBranch()->size().y + shift);
	D3DPaint().draw();
}
