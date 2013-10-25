// ------------------------------------------------------------------------------------------------
//
// UIValueSlider
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "AppBase.h"
#include "UIValueSlider.h"
#include "UIScrollBar.h"
#include "UITextBox.h"
#include "UI/DialogMgr.h"
#include "Standard/DXInput.h"
#include "Standard/Mapping.h"

IMPLEMENT_RTTI(UIValueSlider);

const float BAR_SIZE = 0.0125f;

UIValueSlider::UIValueSlider()
{
	m_sensitivity = DialogMgr().pointerSensitivity();

	m_text = addChild(new UITextBox);
	m_text->onFocusLost = delegate(&UIValueSlider::onText);

	m_bar = addChild(new UIScrollBar);
	m_bar->setOnPosChange(delegate(&UIValueSlider::onScroll));
	m_bar->setThickness(BAR_SIZE);
	m_bar->setInputEnabled(false);
	setValue(m_bar->scrollPos());

	m_bDragging = false;
	m_bFractional = true;
	m_bHardLimit = false;

	setExtent(-100, 100);
}

void UIValueSlider::setExtent(float min, float max)
{
	m_bar->setExtent(min, max);
	// size to max number of digits
	if (abs(min) != FLT_MAX && abs(max) != FLT_MAX)
	{
		int digits = (int)log10(std::max(abs(min), abs(max))) + 1;
		
		if (m_bFractional)
			digits += 3;

		clamp(digits, 6, 10);

		m_text->setSize(digits, '0');
	}
	else
		m_text->setSize(10, '0');

	m_bar->setSize(Point2(m_text->size().x, m_bar->size().y));
	setSize(Point2(m_text->size().x, m_text->size().y + m_bar->size().y));
}

void UIValueSlider::setValue(float v)
{
	float clamped = v;
	clamp(clamped, m_bar->min(), m_bar->max());

	m_bar->setPos(clamped);

	if (m_bHardLimit)
		v = clamped;

	if (m_bFractional)
	{
		m_text->setText(std::string("") + v);
	}
	else
	{
		m_text->setText(std::string("") + (int)v);
	}

	m_value = v;
}

void UIValueSlider::onScroll()
{
	setValue(m_bar->scrollPos());
}

void UIValueSlider::onText()
{
	setValue((float)atof(m_text->text().c_str()));
	onChange();
	onChangeFinished();
}

void UIValueSlider::setSize(const Point2& reqSize)
{
	m_text->setSize(reqSize - Point2(0, BAR_SIZE));
	m_bar->setPos(Point2(0, m_text->size().y));
	m_bar->setThickness(BAR_SIZE);
	m_bar->setHorz(reqSize.x);

	Super::setSize(reqSize);
}

bool UIValueSlider::mouseDown(const Point2& p, int button)
{
	if (m_bar->isPointIn(clientToScreen(p)))
	{
		captureMouse();
		DialogMgr().pointerLock(true);
		setUsePointer(true);
		setPointer(0);
		m_bDragging = true;
	}

	return true;
}

bool UIValueSlider::mousePressed(const Point2& p, int button)
{
	if (m_bDragging)
	{
		if (Input().mouseDeltaX())
		{
			float oldVal = m_value;

			float delta = Mapping::linear
				(
					clamp((float)Input().mouseDeltaX(),	-4096.0f, 4096.0f),
					-2048.0f,
					2048.0f,
					-m_bar->range(),
					m_bar->range()
				);
			
			m_value += delta;

			// snap to zero
			if ((oldVal > 0 && m_value < 0) || (oldVal < 0 && m_value > 0))
				m_value = 0;

			if (!m_bFractional && m_value)
			{
				if (delta < 0)
					m_value = floor(m_value);
				else
					m_value = ceil(m_value);
			}

			setValue(m_value);
			onChange();
		}
	}

	return true;
}

bool UIValueSlider::mouseUp(const Point2& p, int button)
{
	setUsePointer(false);
	DialogMgr().pointerLock(false);
	releaseMouse();
	if (m_bDragging)
	{
		onChangeFinished();
		m_bDragging = false;
	}

	return true;
}

void UIValueSlider::_onFontChange()
{
	Super::_onFontChange();
	setExtent(m_bar->min(), m_bar->max());
}

void UIValueSlider::keyDown(int key)
{
	float delta = 0;

	switch (key)
	{
	case VK_LEFT:
		delta = -1;
		break;
	case VK_RIGHT:
		delta = 1;
		break;
	};

	if (!delta)
		return;

	setValue(m_value + delta * (float)AppBase().timer().frame() * keyScrollSpeed());
	onChange();
	onChangeFinished();

	Super::keyDown(key);
}

float UIValueSlider::keyScrollSpeed() const
{
	float time = 1.0f;
	if (Input().key(VK_CONTROL))
		time *= 5.0f;
	if (Input().key(VK_SHIFT))
		time *= 5.0f;

	return m_bar->range() * time;
}
