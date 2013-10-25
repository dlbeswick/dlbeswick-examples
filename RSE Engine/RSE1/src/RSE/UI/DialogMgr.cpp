// ---------------------------------------------------------------------------------------------------------
// 
// SDialogMgr
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DialogMgr.h"
#include "Exception/DialogMgr.h"
#include "Dialog.h"
#include "DialogFrame.h"
#include "AppBase.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include "Render/Scene.h"
#include "Render/Camera.h"
#include "Render/OSWindow.h"
#include "Render/RenderContext.h"
#include "Render/Materials/Material.h"
#include <Standard/Config.h>
#include "Standard/DXInput.h"
#include "Standard/Profiler.h"
#include <Standard/Exception/Base.h>

#include <fstream>

IMPLEMENT_RTTI(UIBranch);

static float POINTER_SIZE = 0.035f;

// constructor
SDialogMgr::SDialogMgr() :
	m_pointerPos(0.5f, 0.375f),
	m_bPointerClamp(true),
	m_bPointerLock(false),
	m_pointerSensitivity(0.002f),
	m_activeOSWindow(0),
	m_context(0)
{
	m_root = new UIContainer;
	m_root->setInheritClipping(false);
	m_root->setSize(Point2::MAX);
	
	m_mainBranch = new UIBranch;
	addUIBranch(m_mainBranch);

	m_activeBranch = m_mainBranch;

	m_camera = new Camera;

	m_doubleClickTime = GetDoubleClickTime();
}


// destructor
SDialogMgr::~SDialogMgr()
{
	m_root.destroy();
	delete m_camera;
	delete m_context;
	freeHash(m_materials);
}

Matrix4 SDialogMgr::makeProjectionMatrix(Point2 origin, const Point2& size)
{
	// ui projection matrix
	Matrix4 m(Matrix4::IDENTITY);
	m(0, 0) = (2 / size.x);
	m(1, 1) = (-2 / size.y);
	m(3, 0) = -1 - (origin.x * m(0,0));
	m(3, 1) = 1 - (origin.y * m(1,1));

	return m;
}

// create
void SDialogMgr::create()
{
	m_camera->setBase(Matrix4::IDENTITY);
	m_camera->setView(Matrix4::IDENTITY);

	// TBD: do this on res change
	float ratio43 = 640.0f / 480.0f;
	float ratio = (float)D3D().screenSize().x / D3D().screenSize().y;
	float sizeX;
	float sizeY;

	if (ratio > 1)
	{
		sizeX = 1.0f / (ratio43 / ratio);
		sizeY = 0.75f;
	}
	else
	{
		sizeX = 1.0f;
		sizeY = 0.75f * (ratio43 / ratio);
	}

	m_camera->setProjection(makeProjectionMatrix(Point2(0, 0), Point2(sizeX, sizeY)));
	m_mainBranch->setSize(Point2(sizeX, sizeY));

	// load pointer
	m_stdTextures["pointer"] = D3D().loadTextureFromResource("pointer", "TGA");
	m_stdTextures["check"] = D3D().loadTextureFromResource("check", "TGA");
}


// update
void SDialogMgr::update(float delta)
{
	Profile("UI Update");

	Super::update(delta);

	// update pointer pos and delta
	Point2 oldPos = m_pointerPos;

	if (!m_bPointerLock)
	{
		m_pointerPos.x += Input().mouseDeltaX() * m_pointerSensitivity;
		m_pointerPos.y += Input().mouseDeltaY() * m_pointerSensitivity;

		if (m_bPointerClamp)
		{
			clamp(m_pointerPos.x, 0.0f, m_activeBranch->size().x);
			clamp(m_pointerPos.y, 0.0f, m_activeBranch->size().y);
		}
	}

	m_pointerDelta = m_pointerPos - oldPos;

	m_root->children().flush();

	UIElement::prepareInput();

	m_root->update(delta);

	// clear tooltip text if we move the mouse
	if (Input().mouseDeltaX() || Input().mouseDeltaY())
		counters.set(m_helpCounter, 0.5f, delegate(&SDialogMgr::onHelpHover));
}


// draw
void SDialogMgr::draw()
{
	for (Dialog::ElementList::iterator i = m_root->children().begin(); i != m_root->children().end(); ++i)
	{
		// branches
		UIElement::ElementList& list = (*i)->children();
		for (Dialog::ElementList::iterator j = list.begin(); j != list.end(); ++j)
		{
			Dialog* dlg = Cast<Dialog>(j->ptr());
	
			if (dlg && dlg->visible())
				dlg->onPreDraw();
		}
	}

	D3DPaint().reset();
	m_root->draw();
	D3DPaint().draw();

	// draw mouse
	drawPointer(m_mainBranch);
}

void SDialogMgr::drawPointer(const PtrGC<UIElement>& branch)
{
	PtrD3D<IDirect3DTexture9> pPointerTex = m_currentPointerTex;

	// pointer only drawn for active branch
	if (branch && branch != m_activeBranch)
		return;

	D3D().setContext(*m_context);
	D3D().scissor(false);

	if (pPointerTex)
	{
		float x = m_pointerPos.x;
		float y = m_pointerPos.y;
		D3DPaint().setFill(RGBA(1,1,1), pPointerTex);
		D3DPaint().quad2D(x, y, x + POINTER_SIZE, y + POINTER_SIZE);
		D3DPaint().draw();
	}
}

// add
Dialog* SDialogMgr::add(const std::string& id)
{
	try
	{
		Dialog* d = (Dialog*)Dialog::newObject(id.c_str());
	
		activeBranch()->addChild(d);

		return d;
	}
	catch(ExceptionBaseNoRegistrant&)
	{
		EXCEPTIONSTREAM(ExceptionDialogMgr(), "No dialog named " << id << " is defined.");
	}
}

// remove
// you will usually just call dialog::close
void SDialogMgr::remove(const PtrGC<Dialog>& pDlg)
{
	m_root->removeChild(pDlg);
}

// stdTexture
PtrD3D<IDirect3DTexture9> SDialogMgr::stdTexture(const std::string& tex)
{
	TextureList::iterator i;

	i = m_stdTextures.find(tex);
	if (i != m_stdTextures.end())
		return i->second;

	return 0;
}

HWND SDialogMgr::activeWindow() const
{
	if (m_activeOSWindow)
		return m_activeOSWindow->hWnd();
	else
		return AppBase().hWnd();
}

Point2 SDialogMgr::gameToScreen(const Point2& units)
{
	HWND h = activeWindow();

	// get pos from windows
	RECT r;
	GetClientRect(h, &r);
	POINT p;
	p.x = (LONG)((units.x / m_activeBranch->size().x) * (r.right - r.left));
	p.y = (LONG)((units.y / m_activeBranch->size().y) * (r.bottom - r.top));
	ClientToScreen(h, &p);

	return Point2((float)p.x, (float)p.y);
}

Point2 SDialogMgr::screenToGame(Point2 units)
{
	HWND h = activeWindow();

	// get pos from windows
	POINT p;
	p.x = (LONG)units.x;
	p.y = (LONG)units.y;
	ScreenToClient(h, &p);
	RECT r;
	GetClientRect(h, &r);

	return Point2(((float)p.x / (float)(r.right - r.left)) * m_activeBranch->size().x,
					((float)p.y / (float)(r.bottom - r.top)) * m_activeBranch->size().y);
}

void SDialogMgr::setActiveBranch(const PtrGC<UIBranch>& uiBranch)
{
	if (m_activeBranch != uiBranch)
	{
		m_activeBranch->setInputEnabled(true);
		m_activeBranch = uiBranch;
	}
}

void SDialogMgr::setActiveOSWindow(OSWindow* w)
{
	if (!w)
	{
		setActiveBranch(m_mainBranch);
	}
	else
	{
		setActiveBranch(w->uiBranch());
	}

	m_activeOSWindow = w;
	Input().createMouse(activeWindow());

	// enable input only for active dialog
	for (Dialog::ElementList::iterator i = m_root->children().begin(); i != m_root->children().end(); ++i)
	{
		(*i)->setInputEnabled(*i == m_activeBranch);
	}
}

void SDialogMgr::onOSWindowDestroyed(OSWindow* w)
{
	if (w == m_activeOSWindow)
		setActiveOSWindow(0);
}

void SDialogMgr::addUIBranch(const PtrGC<UIBranch>& uiBranch)
{
	m_root->addChild(uiBranch);
	uiBranch->setInheritClipping(false);
}

void SDialogMgr::removeAll()
{
	for (Dialog::ElementList::iterator i = m_root->children().begin(); i != m_root->children().end(); ++i)
		for (Dialog::ElementList::iterator j = (*i)->children().begin(); j != (*i)->children().end(); ++j)
			(*j).destroy();
}

void SDialogMgr::pointerLock(bool bLock)
{
	m_bPointerLock = bLock;
}

void SDialogMgr::pointerClamp(bool bClamp)
{
	m_bPointerClamp = bClamp;
}

void SDialogMgr::onFullscreenToggle(bool bFullscreen)
{
	for (Dialog::ElementList::iterator i = m_root->children().begin(); i != m_root->children().end(); ++i)
	{
		UIElement::ElementList& l = (*i)->children();
		for (Dialog::ElementList::iterator j = l.begin(); j != l.end(); ++j)
		{
			(Cast<Dialog>(*j))->onFullscreenToggle(bFullscreen);
		}
	}
}

PtrGC<UIElement> SDialogMgr::branchFor(UIElement* e)
{
	UIElement* start = e;
	UIElement* lastChild = e;
	while (e && m_root != e)
	{
		lastChild = e;
		e = e->parent().ptr();
	}

	if (lastChild == start)
		return 0;

	return Cast<UIBranch>(lastChild);
}

void SDialogMgr::onMouseOverElement(UIElement* e)
{
	onElementPointerChange(e);
}

void SDialogMgr::onElementPointerChange(UIElement* e)
{
	// set pointer texture
	if (e && e->isMouseOver(m_pointerPos))
	{
		if (e->usePointer())
			m_currentPointerTex = e->pointerTexture();
	}
}

void SDialogMgr::beginDraw()
{
	if (!m_context)
	{
		// setup light
		D3DLIGHT9 l;
		zero(l);
		l.Type = D3DLIGHT_DIRECTIONAL;
		l.Diffuse = RGBA(1,1,1,1);
		l.Specular = RGBA(1,1,1,1);
		l.Direction.z = 1;
		D3DD().SetLight(0, &l);
		D3DD().LightEnable( 0, TRUE);

		D3D().alpha(true);
		D3D().texFilter(true, true);
		D3D().zbuffer(false);

		m_camera->draw();

		D3D().scissor(true);

		m_context = new RenderContext;
	}

	D3D().setContext(*m_context);

	// Duplicated here to support debug option.
	if (!AppBase().options().get<bool>("Debug", "UIClipping"))
	{
		D3D().scissor(false);
	}
}

uint SDialogMgr::doubleClickTime() const
{
	return m_doubleClickTime;
}

void SDialogMgr::onHelpHover()
{
	// find best help hover candidate
	if (UIElement::topMouseOver() && !UIElement::topMouseOver()->help().empty())
	{
		UIElement::topMouseOver()->onHelpHover();
	}
}

const Material& SDialogMgr::material(const std::string& name)
{
	Materials::iterator i = m_materials.find(name);
	if (i != m_materials.end())
		return *i->second;

	return *m_materials.insert(Materials::value_type(name, Cast<Material>(Base::streamFromConfig(AppBase().materials(), name)))).first->second;
}

PtrGC<UIElement> SDialogMgr::mainBranch() const 
{ 
	return m_mainBranch; 
}
