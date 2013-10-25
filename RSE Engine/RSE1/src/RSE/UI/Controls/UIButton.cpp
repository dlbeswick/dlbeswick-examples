// ---------------------------------------------------------------------------------------------------------
// 
// UIButton
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "UIButton.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include "Render/SDeviceD3D.h"

IMPLEMENT_RTTI(UIButtonBase);

static const float BEVEL_SIZE = 0.005f;
static const float DATA_START = 0;
static const float DATA_PRESSEDOFFSET = BEVEL_SIZE * 0.15f;

// Data types

// ButtonText

// constructor
ButtonText::ButtonText(const std::string& text) :
	m_text(text)
{
}


// onDraw
void ButtonText::onDraw()
{
	font().write(D3DPaint(), m_text, 0, 0);
	D3DPaint().draw();
}


// UIButtonBase

// style
UISTYLESTREAM(UIButtonBase)
	STREAMVAR(materialHighlight);
	STREAMVAR(materialShadow);
	STREAMVAR(materialFace);
UISTYLESTREAMEND

template<> void UIButtonBase::Style::onPostRead()
{
	materialHighlightPtr = &material(materialHighlight);
	materialShadowPtr = &material(materialShadow);
	materialFacePtr = &material(materialFace);
}

// constructor
UIButtonBase::UIButtonBase() :
	m_bPressed(false)
{
	setSize(Point2(0.2f, 0.1f));
}

void UIButtonBase::write(obinstream& s) const
{
	Super::write(s);

	// version
	s << 1;
	//s << m_pData;
}

void UIButtonBase::read(ibinstream& s)
{
	Super::read(s);

	// version
	int version;
	s >> version;
	//s >> m_pData;
}

// onDraw
void UIButtonBase::onDraw()
{
	// Draw main background
	D3DPaint().reset();
	D3DPaint().setFill(*style().materialFacePtr);
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();

	const Material* shadowCol(style().materialShadowPtr);
	const Material* brightCol(style().materialHighlightPtr);

	if (m_bPressed)
	{
		std::swap(brightCol, shadowCol);
	}

	Tri t;
	t[0].z = 1;
	t[1].z = 1;
	t[2].z = 1;

	D3DPaint().setFill(*shadowCol);

	// Draw bottom bevel
	t[0].x = BEVEL_SIZE;
	t[0].y = size().y - BEVEL_SIZE;
	t[1].x = t[0].x + size().x - BEVEL_SIZE;
	t[1].y = t[0].y;
	t[2].x = 0;
	t[2].y = t[0].y + BEVEL_SIZE;
	D3DPaint().tri(t);

	t[0].x = size().x - BEVEL_SIZE;
	t[0].y = size().y - BEVEL_SIZE;
	t[1].x = t[0].x + BEVEL_SIZE;
	t[1].y = t[0].y + BEVEL_SIZE;
	t[2].x = 0;
	t[2].y = t[1].y;
	D3DPaint().tri(t);

	// Draw right bevel
	t[0].x = size().x;
	t[0].y = 0;
	t[1].x = t[0].x;
	t[1].y = t[0].y + size().y;
	t[2].x = t[0].x - BEVEL_SIZE;
	t[2].y = t[0].y + size().y - BEVEL_SIZE;
	D3DPaint().tri(t);

	t[0].x = size().x;
	t[0].y = 0;
	t[1].x = t[0].x - BEVEL_SIZE;
	t[1].y = t[0].y + size().y - BEVEL_SIZE;
	t[2].x = size().x - BEVEL_SIZE;
	t[2].y = BEVEL_SIZE;
	D3DPaint().tri(t);
	D3DPaint().draw();

	D3DPaint().setFill(*brightCol);

	// Draw left bevel
	t[0].x = 0;
	t[0].y = 0;
	t[1].x = BEVEL_SIZE;
	t[1].y = BEVEL_SIZE;
	t[2].x = BEVEL_SIZE;
	t[2].y = size().y - BEVEL_SIZE;
	D3DPaint().tri(t);

	t[0].x = 0;
	t[0].y = 0;
	t[1].x = BEVEL_SIZE;
	t[1].y = size().y - BEVEL_SIZE;
	t[2].x = 0;
	t[2].y = size().y;
	D3DPaint().tri(t);

	// Draw top bevel
	t[0].x = 0;
	t[0].y = 0;
	t[1].x = size().x;
	t[1].y = 0;
	t[2].x = size().x - BEVEL_SIZE;
	t[2].y = BEVEL_SIZE;
	D3DPaint().tri(t);

	t[0].x = 0;
	t[0].y = 0;
	t[1].x = size().x - BEVEL_SIZE;
	t[1].y = BEVEL_SIZE;
	t[2].x = BEVEL_SIZE;
	t[2].y = BEVEL_SIZE;
	D3DPaint().tri(t);
	D3DPaint().draw();
}


// onMousePressed
bool UIButtonBase::onMouseDown(const Point2& pos, int button)
{
	if (button == 0)
	{
		setPressed(true);
		return true;
	}

	return false;
}


// onMouseUp
bool UIButtonBase::onMouseUp(const Point2 &pos, int button)
{
	if (button == 0)
	{
		if (m_bPressed)
		{
			setPressed(false);
			onClick();
		}
		return true;
	}

	return false;
}


// mouseOff
void UIButtonBase::mouseOff()
{
	Super::mouseOff();
	setPressed(false);
}

void UIButtonBase::_onFontChange()
{
	Super::_onFontChange();

	if (_fitData && m_pData)
	{
		m_pData->calcSize(size());
		fitToData();
		recalcDataPos(size());
	}
}

void UIButtonBase::setData(ButtonData* pData, bool bFit)
{
	_fitData = bFit;

	if (m_pData)
		removeChild(m_pData);

	m_pData = pData;

	addChild(m_pData);

	m_pData->calcSize(size());

	if (_fitData)
		fitToData();

	recalcDataPos(size());
}

void UIButtonBase::fitToData()
{
	if (m_pData)
		setSize(m_pData->size() + Point2(0.001f, 0.001f));
	else
		dwarn << "UIButtonBase::fitToData called but m_pData is null\n";
}

// setDataPos
void UIButtonBase::recalcDataPos(const Point2& size)
{
	if (!m_pData)
		return;

	m_pData->calcSize(size - Point2(BEVEL_SIZE * 2, BEVEL_SIZE * 2));

	Point2 pos;
	pos.x = (size.x - m_pData->size().x) * 0.5f;
	pos.y = (size.y - m_pData->size().y) * 0.5f;

	if (m_bPressed)
		pos += DATA_PRESSEDOFFSET;

	m_pData->setPos(pos);
}


// setPressed
void UIButtonBase::setPressed(bool bPressed)
{
	if (bPressed != m_bPressed)
	{
		m_bPressed = bPressed;
		recalcDataPos(size());
	}
	else
	{
		m_bPressed = bPressed;
	}
}

// onSize
void UIButtonBase::setSize(const Point2& reqSize)
{
	recalcDataPos(reqSize);

	Super::setSize(reqSize);
}

void UIButtonBase::keyDown(int key)
{
	if (key == VK_SPACE || key == VK_RETURN)
		setPressed(true);
}

void UIButtonBase::keyChar(int key)
{
	if (key == VK_SPACE || key == VK_RETURN)
	{
		onClick();
	}
}

void UIButtonBase::keyUp(int key)
{
	if (key == VK_SPACE || key == VK_RETURN)
	{
		setPressed(false);
	}
}

// UIButton
IMPLEMENT_RTTI(ButtonData);
REGISTER_RTTI(UIButton);
REGISTER_RTTI(ButtonText);

UIButton::UIButton(const std::string& text)
{
	if (!text.empty())
		setText(text);
}

void ButtonText::write(obinstream& s) const 
{
	Super::write(s);

	// version
	s << 1;

	s << m_text;
}

void ButtonText::read(ibinstream& s)
{
	Super::read(s);

	// version
	int version;
	s >> version;

	s >> m_text;
}

void ButtonText::calcSize(const Point2& size)
{
	setSize(Point2(font().stringWidth(m_text), font().height()));
}

// UIButtonPic
REGISTER_RTTI(UIButtonPic);

UIButtonPic::UIButtonPic()
{
}

// ButtonPic
IMPLEMENT_RTTI(ButtonPic);

ButtonPic::ButtonPic(const std::string& fname, const char* resType) :
	m_fname(fname),
	m_resType(resType)
{
	load();
}

void ButtonPic::write(obinstream& s) const
{
	// version
	s << 1;

	s << m_fname;
	//s << m_resType; // fix
}

void ButtonPic::read(ibinstream& s)
{
	// version
	int version;
	s >> version;

	s >> m_fname;
	//s >> m_resType; // fix

	load();
}

void ButtonPic::onDraw()
{
	D3DPaint().setFill(RGBA(1,1,1), m_tex);
	D3DPaint().quad2D(0, 0, size().x, size().y);
	D3DPaint().draw();

	if (!enabled())
	{
		D3DPaint().setFill(RGBA(0, 0, 0, 0.5f));
		D3DPaint().quad2D(0, 0, size().x, size().y);
		D3DPaint().draw();
	}
}

void ButtonPic::load()
{
	if (!m_resType)
		m_tex = D3D().loadTexture(m_fname);
	else
		m_tex = D3D().loadTextureFromResource(m_fname, m_resType);
}
