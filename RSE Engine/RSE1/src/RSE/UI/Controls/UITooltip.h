// ------------------------------------------------------------------------------------------------
//
// UITooltip
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UI/UIElement.h"


class UITooltip : public UIElement
{
	USE_RTTI(UITooltip, UIElement);
public:
	UITooltip(const std::string& text);

protected:
	virtual void onDraw();

	virtual bool onMouseDown(const Point2& p, int button) { self().destroy(); return true; }
	virtual bool onMouseUp(const Point2& p, int button) { self().destroy(); return true; }
	virtual bool onMouseMove(const Point2& p, const Point2& delta) { self().destroy(); return true; }

	void onBranchChange();
};