// ---------------------------------------------------------------------------------------------------------
// 
// UIElement
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/RSE.h"
#include "RSE/UI/Controls/EditableProperties.h"
#include "RSE/UI/Styles/UIStyle.h"
#include "Standard/Base.h"
#include "Standard/PendingList.h"
#include "Standard/PtrD3D.h"
#include "Standard/PtrGCHost.h"
#include "Standard/Registrar.h"
#include "Standard/MultiDelegate.h"

class Config;
class FontElement;
class UIElement;

// UIElement
class RSE_API UIElement : public EditableProperties, public PtrGCHost
{
	USE_RTTI(UIElement, Base);

public:
	typedef PendingListNullRemover<PtrGC<UIElement> >	ElementList;
	typedef std::vector<PtrGC<UIElement> >				Elements;
	typedef TMultiDelegate<UIElement>					MultiDelegate;
	
	virtual ~UIElement();

	// returns pointer to standard dialog root
	static const PtrGC<UIElement>& dlgRoot();

	// properties
	const Point2 &pos() const { return m_pos; }
	virtual void setPos(const Point2 &pos);
	const Point2 &size() const { return m_size; }
	virtual void setSize(const Point2 &s);
	const RGBA &colour() const;
	virtual void setColour(const RGBA &colour);
	
	const Point2 &screenPos() { if (m_xform.dirty()) updateTransform(); return m_screenPos; }
	Point2 screenToClient(const Point2& pos);
	Point2 screenToParent(const Point2& pos);
	Point2 clientToScreen(const Point2& pos);
	Point2 parentToScreen(const Point2& pos);
	bool isPointIn(const Point2& screenpos) const;
	void fitToChildren();
	void getRealScreenExtents(Point2& min, Point2& max);
	
	virtual void setVisible(bool bVisible);
	bool visible() const { return m_bVisible && m_bParentVisible; }
	virtual void setEnabled(bool bEnabled);
	bool enabled() const { return m_bEnabled && m_bParentEnabled; }
	
	// drawOwner -- if another element is our draw owner, we do not draw ourselves
	virtual void setDrawOwner(const PtrGC<UIElement>& who) { m_drawOwner = who; }
	const PtrGC<UIElement>& drawOwner() const { return m_drawOwner; }

	// return true from this function if your control can accept keyboard focus
	virtual bool usesFocus() const { return false; }
	bool canFocus() const;

	// search element and children for suitable focus
	virtual PtrGC<UIElement> findFocus() const;

	void setFont(const FontElement& pFont);
	void setFont(const std::string& fontName);

	// Returns the font that this object uses when it has the given parent.
	virtual const FontElement& font() const;

	// update
	virtual void preUpdate() {}
	virtual void update(float delta);
	virtual void postUpdate() {}
	void updateTransform();
	const Matrix4 &xForm() { if (m_xform.dirty()) updateTransform(); return *m_xform; }

	bool updateWhenHidden;

	// drawing
	virtual void draw();
	virtual void bringToFront(PtrGC<UIElement> what = 0);
	virtual void drawAt(const Point2& screen, Point2 minClip = Point2(FLT_MAX, 0), Point2 maxClip = Point2(FLT_MAX, 0));
	virtual void drawByOwner(const PtrGC<UIElement>& owner);

	void setInheritClipping(bool b) { m_bInheritClipping = b; }
	bool inheritClipping() const { return m_bInheritClipping; }

	virtual Point2 minClip() const;
	virtual Point2 maxClip() const;

	// style
	virtual void setStyle(class UIStyle* style);
	virtual void setStyle(const std::string& styleName);

	// hierarchy
	const PtrGC<UIElement>& parent() const { return m_pParent; }
	template <class T> T* addChild(T* pElement);
	template <class T> T* addChild(const PtrGC<T>& pElement);
	virtual UIElement* _addChild(const PtrGC<UIElement>& pElement);
	virtual void removeChild(const PtrGC<UIElement>& pElement);
	ElementList& children() { return m_children; }
	const ElementList& children() const { return m_children; }
	bool isDescendantOf(const PtrGC<UIElement>& pElement) const;

	PtrGC<UIElement> uiBranch();

	// alignment
	void ensureInRect(const Point2 &screenPos, const Point2 &size, Point2 min = Point2::MAX, Point2 max = Point2::MAX);

	// input
	static void prepareInput();

	virtual bool isMouseOver(const Point2& screenPos);

	void setInputEnabled(bool bEnabled);
	bool inputEnabled() const { return m_bInput && m_bParentInputEnabled; }

	virtual bool acceptMouseOver() const { return true; }
	virtual void mouseOver();
	virtual void mouseOff();

	virtual bool mouseMove(const Point2 &pos, const Point2 &delta) { return onMouseMove(pos, delta); };
	virtual bool mouseDown(const Point2 &pos, int button) { return onMouseDown(pos, button); };
	virtual bool mousePressed(const Point2 &pos, int button) { return onMousePressed(pos, button); };
	virtual bool mouseUp(const Point2 &pos, int button) { return onMouseUp(pos, button); };
	virtual bool mouseDouble(const Point2 &pos, int button) { return false; }

	virtual void keyDown(int key) { onKeyDown(key); };
	virtual void keyPressed() { onKeyPressed(); };
	virtual void keyUp(int key) {}
	virtual void keyChar(int key) { onKeyChar(key); };

	static const PtrGC<UIElement>& focusedElement() { return m_pFocus; }
	static void clearFocus();
	virtual bool focused() const { return m_pFocus == this; }
	virtual void setFocus();

	static const PtrGC<UIElement>& capturedElement() { return m_pMouseCapture; }
	static void releaseMouse() { m_pMouseCapture = 0; }
	bool captured() const;
	void captureMouse() { m_pMouseCapture = this; }

	// pointer
	virtual void setPointer(PtrD3D<IDirect3DTexture9> pTex);
	virtual PtrD3D<IDirect3DTexture9> pointerTexture()		{ return m_pointerTex; }
	virtual void setUsePointer(bool b)						{ m_usePointer = b; }
	bool usePointer() const									{ return m_usePointer; }

	static const PtrGC<UIElement>& topMouseOver()			{ return m_mouseOver; }

	// alignment
	enum ALIGN
	{
		ALIGN_NONE,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER,
	};

	enum VALIGN
	{
		VALIGN_NONE,
		VALIGN_TOP,
		VALIGN_BOTTOM,
		VALIGN_CENTER,
	};

	/// Aligns the ui element using the given parameters, within the context of the given uielemetn.
	/// If context is 0, the ui element's parent is used as the context.
	void align(ALIGN a, VALIGN v, PtrGC<UIElement> context = 0);
	void align(ALIGN a, VALIGN v, const Point2& min, const Point2& max);

	// tooltip help
	virtual void setHelp(const std::string& help) { m_help = help; }
	virtual const std::string& help() const { return m_help; }
	void onHelpHover();

	// streams
	//virtual void write(obinstream& s) const;
	//virtual void read(ibinstream& s);
	void writeChildren(obinstream& s) const;
	void readChildren(ibinstream& s);
	static UIElement* constructFromStream(ibinstream& s);	// this operator creates a new UIElement of the required type

	// delegates
	Delegate		onFocusLost;
	MultiDelegate	onSizeChanged;

	/// When "whole pixel positioning" is set, then the position of the ui element is rounded to the nearest whole pixel coordinate.
	virtual void setWholePixelPositioning(bool b);

protected:
	UIElement(); // can't instatiate UIElement
	virtual void construct();

	class RSE_API Style : public UIStyle
	{
	public:
		std::string fontName;

	protected:
	   virtual void streamVars(StreamVars& v);
	};

	class UIStyle& _style() const;
	virtual std::string styleCategoryOverride() const;

	virtual Config& configUIStyles() const;
	virtual void setParent(const PtrGC<UIElement>& pParent);

	// events
	virtual void onFocus(bool bFocus) {}
	virtual bool onPos(const Point2& newPos) { return true; }
	virtual void onParent() {}
	virtual void onVisible(bool bVisible) {}
	public: virtual void _onFontChange() {} protected:
		// called when one of our children is removed and assigned to a new parent
	virtual void onChildParentChanged(const PtrGC<UIElement>& child, const PtrGC<UIElement>& newParent) {}

	// drawing
	virtual void performDraw(const PtrGC<UIElement>& initiator);
	virtual void preBeginDraw() {}
	virtual void onDraw() {}
	virtual void onPostDraw() {}

	// input
	void processInput();
	virtual void relinquishFocus();

	virtual bool onMouseMove(const Point2 &p, const Point2 &delta) { return false; }
	virtual bool onMouseDown(const Point2 &p, int button) { return false; }
	virtual bool onMousePressed(const Point2 &p, int button) { return false; }
	virtual bool onMouseUp(const Point2 &p, int button) { return false; }
	virtual void onKeyDown(int key) {}
	virtual void onKeyPressed() {}
	virtual void onKeyChar(int key) {}

	virtual void internalMouseOver();
	virtual void internalMouseOff();

	// style
	virtual void createStyle() const;
	static const class Material& material(const std::string& name);

	// convenience / helper
	void nullMethod() {}

	ElementList						m_children;

	bool							_constructed;
	bool							m_bVisible;
	bool							m_bEnabled;
	bool							m_bInput;
	bool							m_bWasMouseOver;
	bool							m_bWasMouseDown;
	PtrGC<UIElement>				m_pParent;
	mutable Dirty<Matrix4>			m_xform;		// updated during drawing

	mutable class UIStyle*			m_style;

	static bool						m_bMousePressedRecord[/*DXInput::MOUSE_BUTTONS*/8];

	virtual void streamVars(StreamVars& v);

private:
	friend class Dialog; // TODO: yukkkk, for userSettings. fix?

	static void doFocus(const PtrGC<UIElement>& pFocus);
	virtual void notifyChildrenVisible();
	virtual void setParentInputEnabled(bool b);
	virtual void setParentVisible(bool b);
	virtual void setParentEnabled(bool b);
	virtual void applyClipping();
	virtual void applyClipping(const Point2& minClip, const Point2& maxClip);
	virtual void calculateClipArea();
	
	void processRealScreenExtents(Point2& min, Point2& max);

	// state
	mutable Point2 m_screenPos;
	Point2 m_pos; // careful! remember to call updateTransform
	Point2 m_size;	// total bound of the object
	RGBA m_colour;	// colour, interpreted by each derived control to suit
	RGBA m_disabledColour;
	mutable const FontElement* _font;
	const FontElement* _lastFont;
	bool m_bVisibleChanged;
	bool m_bParentInputEnabled;
	bool m_bParentVisible;
	bool m_bParentEnabled;
	bool m_bInheritClipping;
	PtrGC<UIElement> m_drawOwner;
	mutable Dirty<Point2> m_minClip;
	mutable Dirty<Point2> m_maxClip;

	// input
	DWORD m_lastMouseClickTime;
	Point2 m_lastMouseClickPos;

	std::string m_help;

	PtrD3D<IDirect3DTexture9>	m_pointerTex;
	bool						m_usePointer;

	static PtrGC<UIElement> m_pFocus;
	static PtrGC<UIElement> m_pMouseCapture;
	static PtrGC<UIElement> m_mouseOver;
	static bool m_mouseOverAvailable;

	bool _wholePixelPositioning;
};


class RSE_API UIElementTransient : public UIElement
{
	USE_RTTI(UIElementTransient, UIElement);

	virtual void write(obinstream& s) const {}
	virtual void read(ibinstream& s) {}
};


class RSE_API UIContainer : public UIElement
{
	USE_RTTI(UIContainer, UIElement);
public:
	virtual bool acceptMouseOver() const { return false; }
};

template <class T> T* UIElement::addChild(T* pElement)
{
	return (T*)_addChild(pElement);
}

template <class T> T* UIElement::addChild(const PtrGC<T>& pElement)
{
	return (T*)_addChild(pElement);
}

// style helpers
#define UISTYLE \
	template <class SuperClass> \
	class RSE_API_SHARED TStyle : public SuperClass::Style \
	{ \
	protected: \
		virtual void streamVars(StreamVars& v); \
	public: \
		typedef typename SuperClass::Style Super;

#define UISTYLEEND \
	}; \
	typedef TStyle<Super> Style; \
	virtual void createStyle() const { m_style = new Style; } \
	Style& style() { return (Style&)_style(); }


#define UISTYLESTREAM(x) template<> void x::Style::streamVars(StreamVars& v) \
	{ \
		Super::streamVars(v);
#define UISTYLESTREAMEND }
