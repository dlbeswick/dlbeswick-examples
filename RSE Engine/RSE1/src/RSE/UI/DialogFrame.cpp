// ------------------------------------------------------------------------------------------------
//
// DialogFrame
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DialogFrame.h"
#include "Dialog.h"
#include "DialogMgr.h"
#include "DragHelper.h"
#include "Render/D3DPainter.h"
#include "Render/FontElement.h"
#include "Render/OSWindow.h"
#include "Render/Materials/MaterialSolid.h"
#include "UI/UILayout.h"
#include "UI/Controls/UIButton.h"
#include "UI/Controls/UIPic.h"
#include "UI/Controls/UIScrollBar.h"
#include "UI/Styles/UIStyle.h"
#include <Standard/DXInput.h>

IMPLEMENT_RTTI(DialogFrame);

DialogFrame::DialogFrame(Dialog* d)
{
	m_scrollHorz = 0;
	m_scrollVert = 0;
}

void DialogFrame::setParent(const PtrGC<UIElement>& pParent)
{
	Super::setParent(pParent);

	m_parentDialog = Cast<Dialog>(parent());
	if (!m_parentDialog)
		throwf("Attempt to add a dialogframe to an element that is not a dialog");

	m_clientArea = &m_parentDialog->clientArea();
	
	onClientAreaSet();

	clientArea().setPos(Point2::ZERO);
}

UIElement& DialogFrame::clientArea()
{
	return *m_clientArea;
}

void DialogFrame::onClientAreaSet()
{
}

DialogFrame::~DialogFrame()
{
}

float DialogFrame::scrollSize()
{
	return UIScrollBar::STD_SIZE;
}

void DialogFrame::update(float delta)
{
	Super::update(delta);

	if (!m_clientArea || !m_parentDialog)
		return;

	setSize(m_parentDialog->size());

	if (clientArea().size().x > m_parentDialog->size().x)
	{
		if (!m_scrollHorz)
		{
			m_scrollHorz = addChild(new UIScrollBar);
			m_scrollHorz->setPage(0.2f);
		}
		m_scrollHorz->setVisible(false);
	}
	else
	{
		if (m_scrollHorz)
		{
			m_scrollHorz->setVisible(false);
		}
	}

	if (clientArea().size().y > m_parentDialog->size().y)
	{
		if (!m_scrollVert)
		{
			m_scrollVert = addChild(new UIScrollBar);
			m_scrollVert->setPage(0.2f);
		}
		m_scrollVert->setVisible(true);
	}
	else
	{
		if (m_scrollVert)
		{
			m_scrollVert->setVisible(false);
		}
	}

	if (m_scrollHorz)
	{
		m_scrollHorz->setExtent(0, clientArea().size().x);
	}

	if (m_scrollVert)
	{
		m_scrollVert->setExtent(0, clientArea().size().y);
	}
}

Point2 DialogFrame::sizeNeededForClient(Point2 request)
{
	return request;
}

///////////////////////////////

REGISTER_RTTI_ARG1(DialogFrameNaked,Dialog*);

DialogFrameNaked::DialogFrameNaked(Dialog* d) :
	Super(d),
	m_needsCalcSize(true)
{
}

void DialogFrameNaked::construct()
{
	Super::construct();

	m_material = new MaterialSolid;
	m_background = addChild(new UIPic(*m_material));

	m_clientContainer = addChild(new UIContainer);
	m_clientContainer->setPos(Point2::ZERO);
}

void DialogFrameNaked::onClientAreaSet()
{
	m_clientContainer->addChild(m_clientArea);
}

DialogFrameNaked::~DialogFrameNaked()
{
	delete m_material;
}

void DialogFrameNaked::calcSizes()
{
	if (!m_needsCalcSize)
		return;

	m_needsCalcSize = false;

	m_background->setSize(size());
	m_clientContainer->setSize(size());

	if (m_scrollHorz)
	{
		m_scrollHorz->setPos(Point2(0, size().y));
		m_scrollHorz->setSize(Point2(size().x, m_scrollHorz->size().y));
		clientArea().setPos(Point2(-m_scrollHorz->scrollPos(), clientArea().pos().y));
	}

	if (m_scrollVert)
	{
		m_scrollVert->setPos(Point2(size().x - m_scrollVert->size().x, 0));
		m_scrollVert->setSize(Point2(m_scrollVert->size().x, size().y));
		clientArea().setPos(Point2(clientArea().pos().x, -m_scrollVert->scrollPos()));
	}
}

void DialogFrameNaked::onDraw()
{
	m_material->diffuse = parent()->colour();
	
	if (m_material->diffuse.a == 0.0f)
		m_background->setVisible(false);

	calcSizes();
}

void DialogFrameNaked::update(float delta)
{
	if (!m_clientArea)
		return;

	Super::update(delta);

	// hack -- fix one frame where sizes are not calculated
	m_needsCalcSize = true;
	calcSizes();
}

///////////////////////////////

REGISTER_RTTI_ARG1(DialogFrameOverlapped,Dialog*);

static const float BORDER_SIZE = 0.005f;
static const float TITLE_SIZE = 0.05f;

DialogFrameOverlapped::DialogFrameOverlapped(Dialog* d) :
	Super(d),
	m_needsCalcSize(true)
{
}

void DialogFrameOverlapped::construct()
{
	Super::construct();

	m_dragger = addChild(new DialogDragHelper());
	m_dragger->setTarget(parent());
	m_dragger->setTolerance(0);

	m_sizer1 = addChild(new DialogDragHelper(true));
	m_sizer1->setTarget(parent());
	m_sizer1->setTolerance(0);

	m_sizer2 = addChild(new DialogDragHelper(true));
	m_sizer2->setTarget(parent());
	m_sizer2->setTolerance(0);

	m_buttonLayout = addChild(new UILayoutGrid);
	m_buttonLayout->setCols(10);
	m_buttonLayout->setFillMethod(false);

	m_close = m_buttonLayout->addChild(new UIButtonPic);
	m_close->setPic("ui_close", RT_BITMAP);
	m_close->setSize(Point2(TITLE_SIZE, TITLE_SIZE));
	m_close->onClick = delegate(&DialogFrameOverlapped::onClose);

	m_osWindow = m_buttonLayout->addChild(new UIButtonPic);
	m_osWindow->setPic("ui_os_window", RT_BITMAP);
	m_osWindow->setSize(Point2(TITLE_SIZE, TITLE_SIZE));
	m_osWindow->onClick = delegate(&DialogFrameOverlapped::onOSWindow);

	m_clientContainer = addChild(new UIContainer);
	m_clientContainer->setPos(Point2(BORDER_SIZE, TITLE_SIZE));
}

void DialogFrameOverlapped::onClientAreaSet()
{
	m_clientContainer->addChild(&clientArea());
	clientArea().setPos(Point2::ZERO);
}

void DialogFrameOverlapped::onDialogStatusChange()
{
	if (!m_parentDialog->canUserClose())
	{
		if (m_close->parent())
			m_close->parent()->removeChild(m_close);
	}
	else
	{
		if (!m_close->parent())
			m_buttonLayout->addChild(m_close);
	}

	setSize(size());
}

void DialogFrameOverlapped::update(float delta)
{
	Super::update(delta);

	// hack -- fix one frame where sizes are not calculated
	m_needsCalcSize = true;
	calcSizes();
}

void DialogFrameOverlapped::calcSizes()
{
	if (!m_needsCalcSize)
		return;

	m_needsCalcSize = false;

	Point2 containerSize = Point2(size().x - BORDER_SIZE * 2, size().y - BORDER_SIZE - TITLE_SIZE);

	if (m_scrollHorz)
	{
		m_scrollHorz->setPos(Point2(0, size().y - BORDER_SIZE));
		m_scrollHorz->setSize(Point2(size().x, m_scrollHorz->size().y));
		clientArea().setPos(Point2(-m_scrollHorz->scrollPos(), clientArea().pos().y));
		containerSize.y -= m_scrollHorz->size().y;
	}

	if (m_scrollVert)
	{
		m_scrollVert->setPos(Point2(size().x - m_scrollVert->size().x - BORDER_SIZE, TITLE_SIZE));
		m_scrollVert->setSize(Point2(m_scrollVert->size().x, size().y - BORDER_SIZE - TITLE_SIZE));
		clientArea().setPos(Point2(clientArea().pos().x, -m_scrollVert->scrollPos()));
		containerSize.x -= m_scrollVert->size().x;
	}

	m_clientContainer->setSize(containerSize);
}

void DialogFrameOverlapped::onDraw()
{
	calcSizes();

	if (m_dragger->dragging())
		setColour(RGBA(1, 1, 1, 0.8f));
	else
		setColour(RGBA(0, 0, 0, 0.5f));

	D3DPaint().setFill(colour());

	// draw left border
	D3DPaint().quad2D(0, TITLE_SIZE, BORDER_SIZE, size().y);

	// draw right border
	D3DPaint().quad2D(size().x - BORDER_SIZE, TITLE_SIZE, size().x, size().y);

	// draw bottom border
	D3DPaint().quad2D(0, size().y - BORDER_SIZE, size().x, size().y);
	
	// draw title
	D3DPaint().quad2D(0, 0, size().x, TITLE_SIZE);
	D3DPaint().draw();

	font().write(D3DPaint(), m_parentDialog->title(), 0, 0);
	D3DPaint().draw();
}

void DialogFrameOverlapped::setSize(const Point2& reqSize)
{
	m_dragger->setSize(Point2(reqSize.x, TITLE_SIZE));
	
	m_sizer1->setPos(Point2(reqSize.x - 0.001f, reqSize.y - BORDER_SIZE));
	m_sizer1->setSize(Point2(0.001f, BORDER_SIZE));
	m_sizer2->setPos(Point2(reqSize.x - BORDER_SIZE, reqSize.y - 0.001f));
	m_sizer2->setSize(Point2(BORDER_SIZE, 0.001f));
	
	m_buttonLayout->setSize(Point2(reqSize.x - BORDER_SIZE * 2, TITLE_SIZE));
	
	Super::setSize(reqSize);
}

Point2 DialogFrameOverlapped::sizeNeededForClient(Point2 request)
{
	request.x += BORDER_SIZE * 2;
	request.y += BORDER_SIZE + TITLE_SIZE;

	return request;
}

void DialogFrameOverlapped::onClose()
{
	m_parentDialog->close();
}

void DialogFrameOverlapped::onOSWindow()
{
	m_parentDialog->setOSManaged(!m_parentDialog->osManaged());
}
