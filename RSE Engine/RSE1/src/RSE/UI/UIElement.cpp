// ---------------------------------------------------------------------------------------------------------
// 
// UIElement
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "UIElement.h"
#include "DialogFrame.h"
#include "DialogMgr.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/FontElement.h"
#include "Render/SFont.h"
#include "Render/SDeviceD3D.h"
#include "Standard/Config.h"
#include "Standard/DXInput.h"
#include "Standard/Rect.h"
#include "Controls/UITooltip.h"
#include "Styles/UIStyle.h"

IMPLEMENT_RTTI(UIElement);
IMPLEMENT_RTTI(UIElementTransient);
REGISTER_RTTI(UIContainer);

// static
PtrGC<UIElement>	UIElement::m_pFocus;
PtrGC<UIElement>	UIElement::m_pMouseCapture;
bool				UIElement::m_bMousePressedRecord[DXInput::MOUSE_BUTTONS];
PtrGC<UIElement>	UIElement::m_mouseOver;

void UIElement::streamVars(StreamVars& v)
{
	Super::streamVars(v);
	STREAMVAR(m_size);
	STREAMVAR(m_colour);
	STREAMVAR(m_disabledColour);
	STREAMVAR(m_bInput);
	STREAMVAR(m_bEnabled);
	STREAMVAR(m_bVisible);
	STREAMVAR(m_pos);
}

void UIElement::Style::streamVars(StreamVars& v)
{
	UIStyle::streamVars(v);
	STREAMVAR(fontName);
}

void UIElement::writeChildren(obinstream& s) const
{
	// write non-transient children
/*	uint numChildren = 0;
	for (uint i = 0; i < m_children.size(); i++)
	{
		if (!Cast<UIElementTransient>(m_children[i]))
			numChildren++;
	}
	s << numChildren;
	for (uint i = 0; i < m_children.size(); i++)
		if (!Cast<UIElementTransient>(m_children[i]))
			m_children[i].writeMaster(s);*/
}

void UIElement::readChildren(ibinstream& s)
{
	// read children
	/*uint numChildren;
	s >> numChildren;
	for (uint i = 0; i < numChildren; i++)
	{
		UIElement* newElem = construct(s);
		PtrGC<UIElement>::readMaster(s, newElem);
		addChild(newElem);
	}*/
}

// this operator creates a new UIElement of the required type
UIElement* UIElement::constructFromStream(ibinstream& s)
{
	std::string className;
	s >> className;
	
	UIElement* e = (UIElement*)UIElement::newObject(className.c_str());

	if (!e)
		throwf("Couldn't make a new UIElement of type '" + className + "' while loading");

	return e;
}


// construct
UIElement::UIElement() : 
	_constructed(false),
	m_pos(0,0), m_size(0,0), m_bVisible(true), 
	m_pParent(0),	// m_pParent should already be set from placement new
	m_bInput(true), m_bWasMouseOver(false),
	m_bWasMouseDown(false), m_bEnabled(true), m_bVisibleChanged(false),
	_font(0),
	_lastFont(0),
	m_bInheritClipping(true),
	m_pointerTex(0),
	m_usePointer(false),
	updateWhenHidden(false),
	m_style(0),
	_wholePixelPositioning(false)
{
	setColour(RGBA(0, 0, 0, 0.5f));

	m_bParentInputEnabled = true;
	m_bParentVisible = true;
	m_bParentEnabled = true;
};


// destruct
UIElement::~UIElement()
{
	m_children.flush();
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		i->destroy();
}

void UIElement::applyClipping()
{
	applyClipping(minClip(), maxClip());
}

void UIElement::applyClipping(const Point2& minClip, const Point2& maxClip)
{
	Point3 minClip3(minClip.x, minClip.y, 0);
	Point3 maxClip3(maxClip.x, maxClip.y, 0);

	minClip3 *= DialogMgr().camera().projection();
	maxClip3 *= DialogMgr().camera().projection();

	David::Rect rect(minClip3.xy(), maxClip3.xy());

	D3D().setScissorRegion(rect);
}

void UIElement::calculateClipArea()
{
	if (m_size == Point2::ZERO)
	{
		m_minClip = m_maxClip = Point2::ZERO;
	}
	else
	{
		Point2 sp = screenPos();
		m_minClip = sp;
		m_maxClip = sp + size();
	}

	// also clip to parent planes if not root
	if (parent() && m_bInheritClipping)
	{
		David::Rect r(*m_minClip, *m_maxClip);
		David::Rect p(parent()->minClip(), parent()->maxClip());

		David::Rect x = r.intersection(p);

		m_minClip = x.min();
		m_maxClip = x.max();

		assert(m_minClip->x <= m_maxClip->x && m_minClip->y <= m_maxClip->y);
	}

	m_minClip.setDirty(false);
	m_maxClip.setDirty(false);
}

void UIElement::createStyle() const
{
	m_style = new Style;
}

UIStyle& UIElement::_style() const
{
	assert(m_style);
	return *m_style;
}

void UIElement::setStyle(UIStyle* style)
{
	if (m_style)
		delete m_style;
	
	m_style = style;
}

void UIElement::setStyle(const std::string& styleName)
{
	setStyle(0);
	createStyle();

	std::string configCategory;

	if (styleName.empty())
		configCategory = styleCategoryOverride();
	else
		configCategory = styleName;
	
	try
	{
		m_style->read(AppBase().uiStyles(), rtti(), configCategory);
	}
	catch (const Exception& e)
	{
		derr << "Exception while loading style for '" << rtti().className() << "': " << e.what() << "\n";
	}
}

std::string UIElement::styleCategoryOverride() const
{
	return configCategory();
}

// draw
void UIElement::draw()
{
	performDraw(0);
}

void UIElement::performDraw(const PtrGC<UIElement>& initiator)
{
	if (!m_bVisible || drawOwner() != initiator)
		return;

	preBeginDraw();

	DialogMgr().beginDraw();

	// performance compromise -- dirty clipping info every frame. otherwise we'll have to iterate
	// through children each time a parent's size/position changes
	m_minClip.setDirty();
	m_maxClip.setDirty();

	applyClipping();

	D3D().zbuffer(false);

	if (AppBase().options().get<bool>("Debug", "UIScreenspace"))
	{
		Matrix4 id;
		id.identity();
		D3D().device().SetTransform(D3DTS_WORLDMATRIX(0), id);
		D3DPaint().setFill(RGBA(1.0f, 1.0f, 1.0f, 1.0f));
		D3DPaint().quad2D(m_screenPos.x, m_screenPos.y, m_screenPos.x + 0.1f, m_screenPos.y + 0.1f);
		D3DPaint().draw();
	}
	
	D3D().device().SetTransform(D3DTS_WORLDMATRIX(0), xForm());

	onDraw();

	// adjust clipping if changed during onDraw
	if (m_minClip.dirty() || m_maxClip.dirty())
		applyClipping();

	// draw elements (back to front)
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->draw();
	}

	D3D().device().SetTransform(D3DTS_WORLDMATRIX(0), xForm());
	applyClipping();

	onPostDraw();
}

void UIElement::drawByOwner(const PtrGC<UIElement>& owner)
{	
	performDraw(owner);
}

void UIElement::drawAt(const Point2& screen, Point2 minClip, Point2 maxClip)
{
	if (minClip.x == FLT_MAX)
	{
		applyClipping();
	}
	else
	{
		applyClipping(minClip, maxClip);
	}

	Matrix4 m;
	m.identity();
	m.translation(screen.x, screen.y, 0);

	D3D().device().SetTransform(D3DTS_WORLDMATRIX(0), m);

	onDraw();

	// draw elements
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		UIElement &el = **i;
		
		el.drawAt(screen + el.pos(), minClip, maxClip);
	}
}

// update
void UIElement::update(float delta)
{
	if (!updateWhenHidden && !visible())
		return;

	Super::update(delta);

	// update children
	m_children.flush();

	// update in reverse order - last drawn, first updated
	for (ElementList::reverse_iterator i = m_children.rbegin(); i != m_children.rend(); ++i)
	{
		(*i)->update(delta);
	}

	const FontElement* curFont = &font();
	if (_lastFont != curFont)
	{
		_lastFont = curFont;
	}

	processInput();

	if (m_bVisibleChanged)
	{
		notifyChildrenVisible();
		m_bVisibleChanged = false;
	}
}


// updateTransform
void UIElement::updateTransform()
{
	if (!m_xform.dirty())
		return;

	// update transform
	if (!m_pParent)
		m_xform->identity();
	else
		m_xform = m_pParent->xForm();

	Matrix4 newXForm;
	newXForm.identity();
	newXForm.translation(m_pos.x, m_pos.y, 0);
	*m_xform *= newXForm;

	m_xform.setDirty(false);

	Point3 screenPos3 = m_xform->translationPart();
	m_screenPos(screenPos3.x, screenPos3.y);

	// update children
	struct DoDirty
	{
		void operator()(UIElement& e)
		{
			for (ElementList::iterator i = e.children().begin(); i != e.children().end(); ++i)
			{
				UIElement* el = i->ptr();
				
				el->m_xform.setDirty();
				DoDirty()(*el);
			}
		}
    };

	DoDirty()(*this);
}

void UIElement::setPos(const Point2 &pos)
{
	if (pos == m_pos)
		return;

	if (onPos(pos))
	{
		m_pos = pos;
		m_xform.setDirty();
		m_minClip.setDirty();
		m_maxClip.setDirty();
	}
}

// processInput
// Call this to have the widget process and act on the current input state
void UIElement::processInput()
{
	if (!visible() || !inputEnabled())
		return;

	// mouseOver / mouseOff functionality
	const Point2& pos(DialogMgr().pointerPos());
	bool bMouseOver = captured() || (!capturedElement() && isMouseOver(pos));

	if (!bMouseOver)
	{
		// if mouse moved off us send mouseOff event
		if (m_bWasMouseOver)
		{
			internalMouseOff();
		}
	}
	else
	{
		// calculate top mouseover element
		if (!m_mouseOver && acceptMouseOver())
		{
			if (m_mouseOver && m_mouseOver != this)
				m_mouseOver->internalMouseOff();

			m_mouseOver = this;
		}

		// send mouseOver event if required
		if (!m_bWasMouseOver)
		{
			internalMouseOver();
		}
	}

	if (!enabled())
		return;

	// Send keyboard events
	if (focused())
	{
		for (uint i = 0; i < Input().keyDown().size(); i++)
		{
			int& k = Input().keyDown()[i];
			if (k)
			{
				keyDown(Input().keyDown()[i]);
				k = 0;
			}
		}

		for (uint i = 0; i < Input().keyChar().size(); i++)
		{
			int& k = Input().keyChar()[i];
			if (k)
			{
				keyChar(Input().keyChar()[i]);
				k = 0;
			}
		}

		for (uint i = 0; i < Input().keyUp().size(); i++)
		{
			int& k = Input().keyUp()[i];
			if (k)
			{
				keyUp(Input().keyUp()[i]);
				k = 0;
			}
		}

		keyPressed();
	}

	if (capturedElement() && !captured())
		return;

	if (!bMouseOver)
		return;

	float deltaX = DialogMgr().pointerDelta().x;
	float deltaY = DialogMgr().pointerDelta().y;

	Point2 relativePos = screenToClient(pos);

	if (deltaX || deltaY)
	{
		// mouseMove
		mouseMove(relativePos, Point2(deltaX, deltaY));
	}

	// send mouse button downs
	DXInput::MouseData &data = Input().mouseData();
	for (uint i = 0; i < data.m_size; i++)
	{
		if (!data.m_bActive[i] || (int)data.m_data[i].dwOfs < DIMOFS_BUTTON0 || (int)data.m_data[i].dwOfs > DIMOFS_BUTTON4)
			continue;

		int button = data.m_data[i].dwOfs - DIMOFS_BUTTON0;
		if ((data.m_data[i].dwData & 0x80))
		{
			m_bWasMouseDown = true;

			if (mouseDown(relativePos, button))
			{
				data.m_bActive[i] = false;

				// set focus
				if (button == 0)
				{
					setFocus();
				}
			}

			// double click
			if (data.m_data[i].dwTimeStamp - m_lastMouseClickTime <= DialogMgr().doubleClickTime())
			{
				// double click must be within a 0.001 units of each other
				if ((m_lastMouseClickPos - relativePos).length() < 0.001f)
				{
					if (mouseDouble(relativePos, button))
					{
						data.m_bActive[i] = false;
					}
				}
			}

			m_lastMouseClickPos = relativePos;
			m_lastMouseClickTime = data.m_data[i].dwTimeStamp;
		}
	}

	// Send button held events
	for (uint i = 0; i < DXInput::MOUSE_BUTTONS; i++)
	{
		bool bPressed = m_bMousePressedRecord[i];

		if (bPressed)
		{
			m_bWasMouseDown = true;

			if (mousePressed(relativePos, i))
			{
				m_bMousePressedRecord[i] = false;
			}
		}
	}

	// send mouse button ups
	for (uint i = 0; i < data.m_size; i++)
	{
		if (!data.m_bActive[i] || (int)data.m_data[i].dwOfs < DIMOFS_BUTTON0 || (int)data.m_data[i].dwOfs > DIMOFS_BUTTON4)
			continue;

		int button = data.m_data[i].dwOfs - DIMOFS_BUTTON0;
		if (!(data.m_data[i].dwData & 0x80) && m_bWasMouseDown) // can't have a mousedown without a mouseup
		{
			if (mouseUp(relativePos, button))
			{
				data.m_bActive[i] = false;
				m_bWasMouseDown = false;
			}
		}
	}
}


// setVisible
void UIElement::setVisible(bool bVisible)
{
	bool wasVisible = m_bVisible;

	m_bVisibleChanged = bVisible != m_bVisible;
	m_bVisible = bVisible;

	// relinquish focus when becoming invisible
	if (wasVisible && !bVisible)
	{
		if (focused())
			relinquishFocus();
	}

	// fix (replace with input clear func)
	if (!bVisible && m_bWasMouseOver)
		internalMouseOff();

	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->setParentVisible(bVisible);

	onVisible(visible());
}

// notifyChildrenVisible
void UIElement::notifyChildrenVisible()
{
	onVisible(visible());

	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->notifyChildrenVisible();
}


// ensureInRect
void UIElement::ensureInRect(const Point2 &screenPos, const Point2 &size, Point2 min, Point2 max)
{
	if (!uiBranch())
		return;

	Point2 newPos = screenPos;

	if (min == Point2::MAX)
	{
		min = Point2::ZERO;
		max = uiBranch()->size();
	}

	// test screen coords against screen extents, adjust them if necessary
	newPos.x = std::max(min.x, newPos.x);
	newPos.x = std::min(max.x - size.x, newPos.x);
	newPos.y = std::max(min.y, newPos.y);
	newPos.y = std::min(max.y - size.y, newPos.y);

	// take the new screen coords and convert them to a local transform
	UIElement::setPos(screenToParent(newPos));
}


// setColour
void UIElement::setColour(const RGBA& colour)
{
	m_colour = colour;
	m_disabledColour = colour;
	m_disabledColour.grey();
}


// colour
const RGBA& UIElement::colour() const
{
	if (m_bEnabled)
	{
		return m_colour;
	}
	else
	{
		return m_disabledColour;
	}
}


// setEnabled
// used to use recursive version on check -- messed up the cache, way too slow
void UIElement::setEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->setParentEnabled(bEnabled);
}

void UIElement::setParentEnabled(bool b)
{
	m_bParentEnabled = b;
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->setParentEnabled(b);
}

// used to use recursive version on check -- but had bad cache usage
void UIElement::setInputEnabled(bool bEnabled)
{
	// fix (replace with input clear func)
	if (!bEnabled && m_bWasMouseOver)
		internalMouseOff();

	bool wasEnabled = m_bInput;

	m_bInput = bEnabled;
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		if (*i)
			(*i)->setParentInputEnabled(bEnabled);
	}

	if (wasEnabled && !m_bInput)
	{
		relinquishFocus();
	}
}

void UIElement::setParentInputEnabled(bool b)
{
	bool wasEnabled = inputEnabled();

	m_bParentInputEnabled = b;
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		if (*i)
			(*i)->setParentInputEnabled(b);
	}

	if (wasEnabled && !inputEnabled())
	{
		relinquishFocus();
	}
}

// setFocus
void UIElement::setFocus()
{
	if (!canFocus())
		return;

	doFocus(this);
}


// doFocus
void UIElement::doFocus(const PtrGC<UIElement>& pFocus)
{
	if (m_pFocus)
	{
		m_pFocus->onFocus(false);
		m_pFocus->onFocusLost();
	}

	if (pFocus)
		pFocus->onFocus(true);

	m_pFocus = pFocus;
}


// clearFocus
void UIElement::clearFocus()
{
	doFocus(PtrGC<UIElement>());
}


void UIElement::setParentVisible(bool b)
{
	m_bParentVisible = b;

	// fix (replace with input clear func)
	if (!b && m_bWasMouseOver)
		mouseOff();

	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->setParentVisible(b);
}


// isPointIn
bool UIElement::isPointIn(const Point2& screenpos) const
{
	return pointBox(screenpos, minClip(), maxClip());
}


// screenToClient
Point2 UIElement::screenToClient(const Point2& pos)
{
	return pos - screenPos();
}

// screenToParent
Point2 UIElement::screenToParent(const Point2& pos)
{
	return pos - m_pParent->screenPos();
}

// parentToScreen
Point2 UIElement::parentToScreen(const Point2& pos)
{
	return pos + m_pParent->screenPos();
}

// clientToScreen
Point2 UIElement::clientToScreen(const Point2& pos)
{
	return pos + screenPos();
}


// prepareInput
void UIElement::prepareInput()
{
	// reset topmost mouseOver
	m_mouseOver = 0;

	for (uint i = 0; i < DXInput::MOUSE_BUTTONS; i++)
		m_bMousePressedRecord[i] = Input().mousePressed(i);
}


// fitToChildren
void UIElement::fitToChildren()
{
	Point2 min(FLT_MAX, FLT_MAX);
	Point2 max(-FLT_MAX, -FLT_MAX);

	getRealScreenExtents(min, max);

	min = screenToClient(min);
	max = screenToClient(max);

	setPos(min);
	UIElement::setSize(max - min);
}


// getRealScreenExtents
void UIElement::getRealScreenExtents(Point2& min, Point2& max)
{
	min = Point2::MAX;
	max = -Point2::MAX;

	processRealScreenExtents(min, max);
}

void UIElement::processRealScreenExtents(Point2& min, Point2& max)
{
	children().flush();
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		if ((*i)->visible())
			(*i)->processRealScreenExtents(min, max);
	}

	Point2 screenMin = screenPos();
	Point2 screenMax = screenMin + size();

	min.x = std::min(min.x, screenMin.x);
	min.y = std::min(min.y, screenMin.y);
	max.x = std::max(max.x, screenMax.x);
	max.y = std::max(max.y, screenMax.y);
}


// isMouseOver
bool UIElement::isMouseOver(const Point2& screenPos)
{
	return isPointIn(screenPos);
}

// addChild
UIElement* UIElement::_addChild(const PtrGC<UIElement>& pElement)
{
	if (pElement && pElement->parent() != this)
	{
		if (pElement->parent())
		{
			pElement->parent()->children().remove(pElement);
			pElement->parent()->onChildParentChanged(pElement, this);
		}

		pElement->setParent(this);
		m_children.add(pElement);
	}

	return pElement.ptr();
}

void UIElement::bringToFront(PtrGC<UIElement> what)
{
	if (!parent())
		return;

	if (!what)
	{
		parent()->bringToFront(this);
		return;
	}

	ElementList::iterator i = std::find(m_children.begin(), m_children.end(), what);
	const PtrGC<UIElement>& a = *i;
	if (i != m_children.end())
	{
		m_children.remove(a);
		m_children.add(what);
	}

	if (!Cast<DialogClientArea>(this))
	{
		if (parent())
		{
			parent()->bringToFront(this);
		}
	}
}

void UIElement::removeChild(const PtrGC<UIElement>& pElement)
{
	if (pElement && pElement->parent() == this)
	{
		m_children.remove(pElement);
		if (pElement->parent())
			pElement->parent()->onChildParentChanged(pElement, 0);
		pElement->setParent(0);
	}
}

struct MethodDetectFontChange
{
public:
	MethodDetectFontChange(UIElement* element=0)
	{
		what = element;
		oldFont = &element->font();
		for (UIElement::ElementList::iterator i = element->children().begin(); i != element->children().end(); ++i)
		{
			children.push_back(MethodDetectFontChange(*i));
		}
	}

	void finalise()
	{
		if (oldFont != &what->font())
			what->_onFontChange();

		for (std::list<MethodDetectFontChange>::iterator i = children.begin(); i != children.end(); ++i)
		{
			i->finalise();
		}
	}

	UIElement* what;
	const FontElement* oldFont;
	std::list<MethodDetectFontChange> children;
};

void UIElement::setParent(const PtrGC<UIElement>& pParent)
{
	assert(pParent != this); // should never be parent of self, this will cause stack overflow

	MethodDetectFontChange methodFontChange(this);

	m_pParent = pParent;
	if (pParent)
	{
		m_bParentInputEnabled = pParent->m_bParentInputEnabled;
		m_bParentEnabled = pParent->m_bParentInputEnabled;
		m_bParentVisible = pParent->m_bParentVisible;
	}
	m_xform.setDirty();
	m_minClip.setDirty();
	m_maxClip.setDirty();

	methodFontChange.finalise();

	// refresh children (for visibility/xform)
	for (ElementList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		(*i)->setParent(this);
	}

	if (!_constructed)
		construct();

	onParent();
}

bool UIElement::captured() const
{
	return m_pMouseCapture == this;
}

void UIElement::setFont(const std::string& fontName)
{
	FontElement* f = Font().get(fontName);
	if (!f)
		throwf("No such font called " + fontName);

	setFont(*f);
}

void UIElement::setFont(const FontElement& pFont) 
{ 
	if (_font != &pFont)
	{
		MethodDetectFontChange detectMethod(this);

		_font = &pFont; 

		detectMethod.finalise();
	}
}

const FontElement& UIElement::font() const
{
	if (!_font && m_style)
	{
		const std::string& fontName = ((Style&)_style()).fontName;

		if (!fontName.empty() && Font().has(fontName))
			_font = Font().get(fontName);
	}

	if (!_font)
	{
		if (parent())
			return parent()->font();
		else
			return *Font().get("arial");
	}

	return *_font;
}

void UIElement::onHelpHover()
{
	const std::string& s = help();

	if (!s.empty())
	{
		uiBranch()->addChild(new UITooltip(s));
	}
}

PtrGC<UIElement> UIElement::uiBranch()
{
	return DialogMgr().branchFor(this);
}

void UIElement::setPointer(PtrD3D<IDirect3DTexture9> pTex)
{
	m_pointerTex = pTex;
	DialogMgr().onElementPointerChange(this);
}

void UIElement::mouseOver()
{
	DialogMgr().onMouseOverElement(this);
}

void UIElement::mouseOff()
{
}

// called when an element must give up the focus, but has no specific element to give focus to.
// transfers focus to the most deserving control.
void UIElement::relinquishFocus()
{
	if (!focused())
		return;

	PtrGC<UIElement> focus;
	UIElement* p = parent().ptr();
	while (p)
	{
		// iterate through parent's children in reverse order
		for (ElementList::reverse_iterator i = p->children().rbegin(); i != p->children().rend(); ++i)
		{
			const PtrGC<UIElement>& e = *i;
			if (e && e != this && e->canFocus())
			{
				focus = e;
				break;
			}
		}

		if (focus)
			break;

		p = p->parent().ptr();
	}

	// if no focus was found, search branch for new focus
	if (!focus)
	{
		focus = uiBranch()->findFocus();
	}

	if (focus)
	{
		// hack -- special case, otherwise dialog receives focus whereas we usually want dialog's best control
		// to get focus.
		PtrGC<Dialog> dlg = Cast<Dialog>(focus);
		if (dlg)
		{
			focus = dlg->findFocus();
			if (focus)
			{
				focus->setFocus();
				return;
			}
		}

		focus->setFocus(); 
	}
}

void UIElement::setSize(const Point2 &s)
{
	if (s == m_size)
		return;

	m_size = s;
	onSizeChanged();
	m_minClip.setDirty();
	m_maxClip.setDirty();
}

bool UIElement::isDescendantOf(const PtrGC<UIElement>& pElement) const
{
	for (ElementList::iterator i = pElement->children().begin(); i < pElement->children().end(); ++i)
	{
		UIElement& e = **i;
		if (&e == this || isDescendantOf(this))
			return true;
	}

	return false;
}

void UIElement::internalMouseOff()
{
	m_bWasMouseOver = false;

	mouseOff();

	if (m_mouseOver == this)
		m_mouseOver = 0;
}

void UIElement::internalMouseOver()
{
	m_bWasMouseOver = true;
	mouseOver();
}

bool UIElement::canFocus() const
{
	return visible() && enabled() && inputEnabled() && usesFocus();
}

void UIElement::align(ALIGN a, VALIGN v, PtrGC<UIElement> context)
{
	if (!context)
	{
		context = parent();
		if (!context)
			return;
	}

	align(a, v, context->screenPos(), context->screenPos() + context->size());
}

void UIElement::align(ALIGN a, VALIGN v, const Point2& min, const Point2& max)
{
	Point2 newPos(parent()->clientToScreen(pos()));

	if (a == ALIGN_LEFT)
	{
		newPos.x = min.x;
	}
	else if (a == ALIGN_RIGHT)
	{
		newPos.x = min.x + (max.x - min.x) - size().x;
	}
	else if (a == ALIGN_CENTER)
	{
		newPos.x = min.x + ((max.x - min.x) - size().x) * 0.5f;
	}
	
	if (v == VALIGN_TOP)
	{
		newPos.y = min.y;
	}
	else if (v == VALIGN_BOTTOM)
	{
		newPos.y = min.y + (max.y - min.y) - size().y;
	}
	else if (v == VALIGN_CENTER)
	{
		newPos.y = min.y + ((max.y - min.y) - size().y) * 0.5f;
	}

	setPos(parent()->screenToClient(newPos));
}

const Material& UIElement::material(const std::string& name)
{
	return DialogMgr().material(name);
}

PtrGC<UIElement> UIElement::findFocus() const
{
	// should this be a hard rule? it will prevent focus from moving to invisible objects automatically.
	if (!visible())
		return 0;

	for (ElementList::const_iterator i = children().begin(); i != children().end(); ++i)
	{
		const PtrGC<UIElement>& c = *i;
		
		if (c->canFocus())
		{
			return c;
		}

		PtrGC<UIElement> newResult = c->findFocus();
		if (newResult)
			return newResult;
	}

	if (canFocus())
		return this;
	else
		return 0;
}

Point2 UIElement::minClip() const
{
	if (m_minClip.dirty())
	{
		const_cast<UIElement*>(this)->calculateClipArea();
	}

	return *m_minClip;
}

Point2 UIElement::maxClip() const
{
	if (m_maxClip.dirty())
	{
		const_cast<UIElement*>(this)->calculateClipArea();
	}

	return *m_maxClip;
}

const PtrGC<UIElement>& UIElement::dlgRoot()
{
	return DialogMgr().activeBranch();
}

void UIElement::setWholePixelPositioning(bool b)
{
	_wholePixelPositioning = b;
}

Config& UIElement::configUIStyles() const
{
	return AppBase().uiStyles();
}

void UIElement::construct()
{
	Super::construct();
	_constructed = true;

	if (!m_style)
		setStyle("");
}
