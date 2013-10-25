// ---------------------------------------------------------------------------------------------------------
// 
// UITextBox
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "UITextBox.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include <Render/SFont.h>
#include "Standard/DXInput.h"

static const float BUFFER_SIZE = 0.001f;
static const float BUFFER_SIZE_CHARS = 0.001f;
static const float LABEL_SPACING = 0.01f;

IMPLEMENT_RTTI(UITextBox);

// constructor
UITextBox::UITextBox(const std::string& text) :
	m_cursorStart(0),
	m_cursorEnd(0),
	m_vAlign(VCENTER),
	m_align(LEFT),
	m_chars(25),
	m_bSizeToChars(false),
	m_cursorSelExtension(-1)
{
	setSize(25);
	setText(text);
	setTextColour(RGBA(1,1,1));
}

// onMousePressed
bool UITextBox::onMousePressed(const Point2& pos, int button)
{
	if (button == 0)
	{
		int newPos = posToCursor(pos.x);
		if (newPos < m_cursorClickPos)
			m_cursorStart = newPos;
		else
			m_cursorEnd = newPos;
	}
	return true;
}


// onMouseUp
bool UITextBox::onMouseDown(const Point2& pos, int button)
{
	if (button == 0)
	{
		m_cursorSelExtension = -1;
		m_cursorStart = m_cursorEnd = m_cursorClickPos = posToCursor(pos.x);
	}
	return true;
}


// onKeyDown
void UITextBox::onKeyDown(int key)
{
	switch (key)
	{
	case VK_LEFT:
		if (m_cursorStart > 0)
		{
			if (!Input().key(VK_SHIFT))
			{
				m_cursorSelExtension = -1;
				m_cursorEnd = m_cursorClickPos = --m_cursorStart;
			}
			else
			{
				if (m_cursorSelExtension == -1)
					m_cursorSelExtension = m_cursorStart - 1;
				else
					m_cursorSelExtension--;

				m_cursorStart = std::min(m_cursorClickPos, m_cursorSelExtension);
				m_cursorEnd = std::max(m_cursorClickPos, m_cursorSelExtension);
			}
		}
		break;

	case VK_RIGHT:
		if (m_cursorEnd < (int)m_text[0].size())
		{
			if (!Input().key(VK_SHIFT))
			{
				m_cursorSelExtension = -1;
				m_cursorStart = m_cursorClickPos = ++m_cursorEnd;
			}
			else
			{
				if (m_cursorSelExtension == -1)
					m_cursorSelExtension = m_cursorEnd + 1;
				else
					m_cursorSelExtension++;

				m_cursorStart = std::min(m_cursorClickPos, m_cursorSelExtension);
				m_cursorEnd = std::max(m_cursorClickPos, m_cursorSelExtension);
			}
		}
		break;
	
	case VK_DELETE:
		if (m_cursorStart != m_cursorEnd)
		{
			m_text[0].erase(m_cursorStart, (m_cursorEnd - m_cursorStart) + 1);
			m_cursorEnd = m_cursorStart;
		}
		else
		{
			if (m_cursorStart >= 0)
			{
				m_text[0].erase(m_cursorStart, 1);
			}
		}
		break;
	}
}


// onKeyChar
void UITextBox::onKeyChar(int key)
{
	if (key == 8) // backspace
	{
		if (m_cursorStart != m_cursorEnd)
		{
			m_text[0].erase(m_cursorStart, (m_cursorEnd - m_cursorStart) + 1);
			
			m_cursorEnd = m_cursorStart;
		}
		else
		{
			if (m_cursorStart > 0)
			{
				m_text[0].erase(m_cursorStart - 1, 1);
				m_cursorEnd = --m_cursorStart;
			}
		}
	}
	else if (key == 13)
	{
		clearFocus();
	}
	else
	{
		if (m_cursorStart != m_cursorEnd)
		{
			m_text[0].erase(m_cursorStart, (m_cursorEnd - m_cursorStart) + 1);
			m_cursorEnd = m_cursorStart;
		}

		m_text[0].insert(m_cursorStart, 1, (char)key);
		m_cursorStart = m_cursorClickPos = ++m_cursorEnd;
	}
}


// onDraw
void UITextBox::onDraw()
{
	// draw background
	if (enabled())
	{
		D3DPaint().setFill(colour());
		D3DPaint().quad2D(0, 0, size().x, size().y);
		D3DPaint().draw();
	}

	// draw text
	float y = (size().y - font().height()) * 0.5f;
	if (m_bWrap)
	{
		drawWrappedText();
	}
	else
	{
		drawLine(D3DPaint(), m_text[0], BUFFER_SIZE * 0.5f, y, m_textColour);
	}

	// draw cursor
	if (focused())
	{
		float x = cursorToPos(m_cursorStart);
		float width;

		if (m_cursorStart > (int)m_text[0].size() - 1)
		{
			width = font().stringWidth("u");
		}
		else
		{
			width = font().stringWidth(m_text[0].substr(m_cursorStart, (m_cursorEnd - m_cursorStart) + 1));
		}
		
		D3DPaint().setFill(RGBA(1, 1, 1, 0.5f));
		D3DPaint().quad2D(x, y, x + width, y + font().height());
		D3DPaint().draw();
	}
}


// fitToText
void UITextBox::fitToText()
{
	m_bSizeToChars = false;

	// Split text into lines while ignoring the control's boundaries.
	// This takes into account natural linebreaks in the text.
	splitText(false);

	Point2 newSize = -Point2::MAX;
	newSize.y = m_text.size() * font().height();

	for (uint i = 0; i < m_text.size(); ++i)
		newSize.x = std::max(font().stringWidth(m_text[i]), newSize.x);

	UIElement::setSize(newSize);
}

void UITextBox::setSize(int chars, char templateChar)
{
	UIElement::setSize(
		Point2(
			font().stringWidth(std::string(chars, templateChar)) + BUFFER_SIZE_CHARS, 
			font().height() + BUFFER_SIZE
		)
	);

	m_chars = chars;
	m_bSizeToChars = true;
}

// cursorToPos
float UITextBox::cursorToPos(int cursor) const
{
	if (m_text.empty())
		return 0;

	return font().stringWidth(m_text[0].substr(0, cursor)) + BUFFER_SIZE;
}


// posToCursor
int UITextBox::posToCursor(float pos) const
{
	if (m_text.empty())
		return 0;

	pos -= BUFFER_SIZE;
	if (pos < 0)
		pos = 0;

	// tbd: fix for multiline?
	// tbd: fix for alignment.
	for (uint i = 0; i < m_fullText.size(); i++)
	{
		if (font().stringWidth(m_fullText.substr(0, i+1)) >= pos)
			return i;
	}

	return m_text[0].size();
}

// textStartX
float UITextBox::textStartX(int line) const
{
	switch (m_align)
	{
	case LEFT:
		return BUFFER_SIZE;

	case RIGHT:
		return size().x - BUFFER_SIZE;

	case CENTER:
		return (size().x - font().stringWidth(m_text[line])) * 0.5f;
	}

	return 0;
}

// drawWrappedText
void UITextBox::drawWrappedText()
{
	uint i = 0;
	float x, y;

	switch (m_vAlign)
	{
	case TOP:
		y = BUFFER_SIZE;
		break;

	case BOTTOM:
		y = size().y - BUFFER_SIZE - font().height() * m_text.size();
		break;

	case VCENTER:
		y = (size().y - m_text.size() * font().height()) * 0.5f;
		break;
	}

	for (i = 0; i < m_text.size(); i++)
	{
		x = textStartX(i);

		drawLine(D3DPaint(), m_text[i], x, y, m_textColour);
		y += font().height();
	}
}

void UITextBox::drawLine(class D3DPainter& painter, const std::string text, float x, float y, const RGBA& colour)
{
	font().write(painter, text, x, y, 1.0f, colour);
	D3DPaint().draw();
}

void UITextBox::splitText(bool constrainToControlSize)
{
	std::string& text = m_fullText;
	m_text.clear();

	uint i = 0;

	// separate text into lines
	while (i < text.size())
	{
		float width = 0;
		float maxWidth = size().x - BUFFER_SIZE * 2;
		std::string line;
		bool bCR = false;
		
		while (i < text.size() && !bCR && (!constrainToControlSize || width < maxWidth || line.empty()))
		{
			uint nextToken = std::min(text.find('\n', i), text.find(' ', i));
			float wordWidth;
			std::string word;
			uint wordSize;
			bool bOverflow;

			if (nextToken == std::string::npos)
				nextToken = text.size() - 1;

			word = text.substr(i, (nextToken - i) + 1);
			wordSize = word.size();

			i += wordSize;

			// force carriage return
			if (word[word.size() - 1] == '\n')
			{
				bCR = true;
				word = word.substr(0, word.size() - 1);
			}

			wordWidth = font().stringWidth(word);
			bOverflow = constrainToControlSize && width + wordWidth > maxWidth;

			if (bOverflow)
			{
				// if a trailing space can be cut to make it fit, then do it
				if (word[word.size()-1] == ' ')
				{
					if (width + wordWidth - font().stringWidth(" "))
					{
						// remove space and try again
						word = word.substr(0, word.size() - 1);
						wordWidth = font().stringWidth(word);
						bOverflow = width + wordWidth > maxWidth;
						// if we still overflow, too bad
						if (bOverflow)
						{
							bOverflow = false;
						}
					}
				}
				else
				{
					// we just have to split the word at max width
					uint cutIdx;
					wordWidth = 0;
					
					for (cutIdx = 0; cutIdx < word.size() && wordWidth < maxWidth; ++cutIdx)
					{
						wordWidth += font().stringWidth(std::string(word[cutIdx], 1));
					}
                    
					bCR = true;
					bOverflow = false;
					word = word.substr(0, cutIdx);
					i -= word.size() - cutIdx - 1;
				}
			}
			
			if (!bOverflow)
			{
				line += word;
			}
			else
			{
				i -= wordSize;
			}

			width += wordWidth;
		}

		m_text.push_back(line);
	}

	if (m_text.empty())
		m_text.push_back("");
}

// setText
void UITextBox::setText(const std::string& text)
{
	m_fullText = text;
	
	if (size().x == 0.0f || size().y == 0.0f)
		fitToText();
	else
		splitText();
}

std::string UITextBox::text()
{
	std::string s;
	for (uint i = 0; i < m_text.size(); i++)
	{
		if (i != 0)
			s += '\n';
		
		s += m_text[i];
	}

	return s;
}

void UITextBox::_onFontChange()
{
	if (m_bSizeToChars)
		setSize(m_chars);
}

// UILabel
IMPLEMENT_RTTI(UILabel);

UILabel::UILabel(const std::string& text) :
	UITextBox(text)
{
}

void UILabel::construct()
{
	Super::construct();

	setEnabled(false);
	fitToText();
}


// UILabelTextBox
IMPLEMENT_RTTI(UILabelTextBox);

// constructor
UILabelTextBox::UILabelTextBox()
{
	m_pTextBox = new UITextBox;
	addChild(m_pTextBox);

	setLabel("Label:");
}


// onDraw
void UILabelTextBox::onDraw()
{
	font().write(D3DPaint(), m_label, 0, 0);
	D3DPaint().draw();
}


// setLabel
void UILabelTextBox::setLabel(const std::string& label)
{
	m_label = label;
	float width = font().stringWidth(m_label);

	// make sure size is big enough for label and text box
	UIElement::setSize(Point2(width + LABEL_SPACING + m_pTextBox->size().x, size().y));

	m_pTextBox->setPos(Point2(width + LABEL_SPACING, BUFFER_SIZE * 0.5f));
}
