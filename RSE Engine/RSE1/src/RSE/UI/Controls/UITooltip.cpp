// ------------------------------------------------------------------------------------------------
//
// UITooltip
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UITooltip.h"
#include "UITextBox.h"
#include "Render/D3DPainter.h"
#include "UI/DialogMgr.h"

IMPLEMENT_RTTI(UITooltip);

UITooltip::UITooltip(const std::string& text)
{
	captureMouse();

	setColour(RGBA(0.0f, 0.0f, 0.5f));

	UITextBox* b = addChild(new UITextBox());
	b->setFont("smallarial");
	b->setSize(Point2(1,1));
	b->setText(text);
	b->setColour(colour());
	b->setPos(Point2(0.005f, 0.005f));
	b->fitToText();

	Point2 newSize = b->size() + 0.01f;

	clamp(newSize.x, 0.0f, 1.0f);
	clamp(newSize.y, 0.0f, 1.0f);

	setSize(newSize);

	ensureInRect(DialogMgr().pointerPos() + Point2(0.025f, 0), size());
}

void UITooltip::onDraw()
{
	D3DPaint().reset();
	D3DPaint().setFill(RGBA(1,1,1));
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();
}

void UITooltip::onBranchChange()
{
	// tbd: there must be a better solution than to have to destroy itself when the dialog branch changes.
	// it just has to listen for input, or be tied to the input.
	self().destroy();
}