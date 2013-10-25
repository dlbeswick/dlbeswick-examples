// ------------------------------------------------------------------------------------------------
//
// DialogFrame
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UIElement.h"


class RSE_API DialogClientArea : public UIElementTransient
{
	USE_RTTI(DialogClientArea, UIElementTransient);
public:
	virtual bool mouseDown(const Point2& p, int button) { return false; }
	virtual bool mouseMove(const Point2& p, const Point2& delta) { return false; }

protected:
	virtual bool usesFocus() const { return false; }
};


class RSE_API DialogFrame : public UIElement
{
	USE_RTTI(DialogFrame, UIElement);

public:
	DialogFrame(class Dialog* d);
	virtual ~DialogFrame();

	// returns the dialog size required to fully display a client area of the given size
	virtual Point2 sizeNeededForClient(Point2 request);
	virtual float scrollSize();

	virtual void onDialogStatusChange() {};

	PtrGC<class UIScrollBar> scrollHorz() { return m_scrollHorz; }
	PtrGC<class UIScrollBar> scrollVert() { return m_scrollVert; }

protected:
	virtual UIElement& clientArea();

	virtual void onClientAreaSet();
	virtual void setParent(const PtrGC<UIElement>& pParent);
	virtual void update(float delta);

	PtrGC<class Dialog>			m_parentDialog;
	PtrGC<class UIScrollBar>	m_scrollHorz;
	PtrGC<class UIScrollBar>	m_scrollVert;
	PtrGC<UIElement>			m_clientArea; // owned by dialog
};


class RSE_API DialogFrameNaked : public DialogFrame
{
	USE_RTTI(DialogFrameNaked, DialogFrame);
public:
	DialogFrameNaked(Dialog* d);
	~DialogFrameNaked();
	
protected:
	virtual void construct();
	virtual void onClientAreaSet();
	virtual void update(float delta);
	virtual void onDraw();

	virtual void calcSizes();

	class UIElement*		m_clientContainer;
	class UIPic*			m_background;
	class MaterialSolid*	m_material;

	bool m_needsCalcSize;
};

class RSE_API DialogFrameOverlapped : public DialogFrame
{
	USE_RTTI(DialogFrameOverlapped, DialogFrame);
public:
	DialogFrameOverlapped(Dialog* d);

	virtual void setSize(const Point2& reqSize);

	virtual Point2 sizeNeededForClient(Point2 request);
	virtual void onDialogStatusChange();

protected:
	virtual void construct();
	virtual void onClientAreaSet();
	void onDragStart();
	virtual void calcSizes();

	virtual void update(float delta);
	virtual void onDraw();

	void onClose();
	void onOSWindow();

	PtrGC<class DialogDragHelper>	m_dragger;
	PtrGC<class DialogDragHelper>	m_sizer1;
	PtrGC<class DialogDragHelper>	m_sizer2;
	PtrGC<class UIButtonPic>		m_close;
	PtrGC<class UIButtonPic>		m_osWindow;
	PtrGC<class UILayoutGrid>		m_buttonLayout;

	PtrGC<class UIElement>		m_clientContainer;

	bool m_needsCalcSize;
};
