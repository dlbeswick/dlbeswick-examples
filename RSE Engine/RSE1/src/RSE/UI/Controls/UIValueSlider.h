// ------------------------------------------------------------------------------------------------
//
// UIValueSlider
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"

class UITextBox;
class UIScrollBar;


class RSE_API UIValueSlider : public UIElement
{
	USE_RTTI(UIValueSlider, UIElement);
public:
	UIValueSlider();

	virtual bool usesFocus() const { return true; }

	virtual void setSize(const Point2& reqSize);

	void setExtent(float min, float max);
	void setValue(float v);
	void setFractional(bool b) { m_bFractional = b; }
	void setHardLimit(bool b) { m_bHardLimit = b; }
	void setSensitivity(float f) { m_sensitivity = f; }

	float value() const { return m_value; }

	Delegate onChange;
	Delegate onChangeFinished;

	virtual bool mouseDown(const Point2& p, int button);
	virtual bool mousePressed(const Point2& p, int button);
	virtual bool mouseUp(const Point2& p, int button);

	virtual void keyDown(int key);

protected:
	virtual float keyScrollSpeed() const;

	virtual void _onFontChange();
	void onScroll();
	void onText();

	UITextBox* m_text;
	UIScrollBar* m_bar;

	bool m_bFractional;
	bool m_bHardLimit;
	bool m_bDragging;
	float m_value;
	float m_sensitivity;
};
