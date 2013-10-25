// ------------------------------------------------------------------------------------------------
//
// TransitionFade
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UITransition.h"


class RSE_API TransitionFade : public TransitionRenderToTexture
{
	USE_RTTI(TransitionFade, TransitionRenderToTexture);
public:
	TransitionFade(const PtrGC<UIElement>& src, float time);

	virtual void draw();

protected:
private:
};