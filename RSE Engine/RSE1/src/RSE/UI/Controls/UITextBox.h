// ---------------------------------------------------------------------------------------------------------
// 
// UITextBox
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/UIElement.h"


// UITextBox
class RSE_API UITextBox : public UIElement
{
	USE_RTTI(UITextBox, UIElement);
public:
	UITextBox(const std::string& text = "");

	virtual bool usesFocus() const { return true; }

	virtual void setSize(const Point2& p) { m_bSizeToChars = false; UIElement::setSize(p); }
	void setSize(int chars, char templateChar='a');
	void fitToText();
	void setText(const std::string& text);
	void setTextColour(const RGBA& colour) { m_textColour = colour; }
	
	std::string text();

	void setWrap(bool b) { m_bWrap = b; }	// AKA multiline

	enum ALIGN
	{
		LEFT,
		RIGHT,
		CENTER
	};
	void setAlign(ALIGN a) { m_align = a; }

	enum VALIGN
	{
		TOP,
		BOTTOM,
		VCENTER
	};
	void setVAlign(VALIGN a) { m_vAlign = a; }

protected:
	virtual void onDraw();
	virtual void _onFontChange();

	// input
	virtual bool onMousePressed(const Point2& pos, int button);
	virtual bool onMouseDown(const Point2 &pos, int button);
	virtual void onKeyDown(int key);
	virtual void onKeyChar(int key);

	virtual float cursorToPos(int cursor) const;
	virtual void drawLine(class D3DPainter& painter, const std::string text, float x, float y, const RGBA& colour);
	virtual int posToCursor(float pos) const;
	virtual float textStartX(int line) const;
	virtual void drawWrappedText();

	// if constrainToControlSize is false, then lines are only split when linebreaks are encountered, rather than
	// being split when the text would overflow the control boundaries.
	virtual void splitText(bool constrainToControlSize = true);

	std::string m_fullText;
	typedef std::vector<std::string> Text;
	Text m_text;
	int m_cursorStart;
	int m_cursorEnd;
	int m_cursorClickPos;
	int m_cursorSelExtension;
	int m_mode;
	int m_chars;
	ALIGN m_align;
	VALIGN m_vAlign;
	bool m_bWrap;
	bool m_bSizeToChars;
	RGBA m_textColour;
};


// UILabel
class RSE_API UILabel : public UITextBox
{
	USE_RTTI(UILabel, UITextBox);
public:
	UILabel(const std::string& text = "");

protected:
	virtual void construct();
};

// UILabelTextBox
class RSE_API UILabelTextBox : public UIElement
{
	USE_RTTI(UILabelTextBox, UIElement);
public:
	UILabelTextBox();

	void setText(const std::string& text) { m_pTextBox->setText(text); }
	void setLabel(const std::string& label);
	UITextBox& box() { return *m_pTextBox; }

protected:
	virtual void onDraw();

	std::string m_label;

	UITextBox* m_pTextBox;
};
