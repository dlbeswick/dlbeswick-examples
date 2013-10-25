// ------------------------------------------------------------------------------------------------
//
// OSWindow
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "OSWindow.h"
#include "AppBase.h"
#include "Render/Camera.h"
#include "Render/SDeviceD3D.h"
#include "UI/Dialog.h"
#include "UI/DialogMgr.h"
#include <Standard/DXInput.h>

WNDCLASSEX* OSWindow::m_sharedWndClass = 0;

// UIBranchOSWindow ////////////////////////////////////////
class UIBranchOSWindow : public UIBranch
{
	USE_RTTI(UIBranchOSWindow, UIBranch);
public:
	UIBranchOSWindow(OSWindow& _window);

	virtual void draw();

	OSWindow* window;
};

IMPLEMENT_RTTI(UIBranchOSWindow);

UIBranchOSWindow::UIBranchOSWindow(OSWindow& _window) :
	window(&_window)
{
}

void UIBranchOSWindow::draw()
{
	if (window)
		window->beginDraw();

	if (window)
		Super::draw();

	if (window)
		window->endDraw();
}

// OSWindow ////////////////////////////////////////////////
OSWindow::OSWindow(const PtrGC<Dialog>& d, WNDCLASSEX& wndClass) :
	m_chain(0)
{
	m_wndClass = wndClass;

	// create the OS window.
	m_hWnd = CreateWindow( m_wndClass.lpszClassName, AppBase().name().c_str(), 
							WS_POPUP, 0, 0, 100, 100,
							NULL, NULL, m_wndClass.hInstance, NULL );

	if (!m_hWnd)
		return;

	// add game menu option
	HMENU menu = GetSystemMenu(m_hWnd, false);
	InsertMenu(menu, GetMenuItemCount(menu) - 1, MF_BYPOSITION | MF_STRING, 1, "&Game-Managed Window");
	InsertMenu(menu, GetMenuItemCount(menu) - 1, MF_BYPOSITION | MF_SEPARATOR, 0, "");

	m_uiBranch = new UIBranchOSWindow(*this);
	DialogMgr().addUIBranch(m_uiBranch);

	SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)(DWORD_PTR)this);
	setDialog(d);
}

OSWindow::~OSWindow()
{
	if (m_uiBranch)
		m_uiBranch->window = 0;

	SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)(DWORD_PTR)0);

	DialogMgr().onOSWindowDestroyed(this);

	if (m_chain)
		m_chain->Release();

	if (m_hWnd)
		DestroyWindow(m_hWnd);

	if (m_dialog)
	{
		if (m_dialog->parent())
			m_dialog->parent()->removeChild(m_dialog);

		DialogMgr().activeBranch()->addChild(m_dialog);
	}

	m_uiBranch.destroy();
}

void OSWindow::setDialog(const PtrGC<Dialog>& d)
{
	m_dialog = d;

	const Matrix4 prjDialog = DialogMgr().camera().projection();

	Point2i cSize = clientSize();

	// size and position window
	Point2i desiredSize((int)(d->size().x * D3D().screenSize().x), (int)(d->size().y * D3D().screenSize().x));
	Point2 screenPos = DialogMgr().gameToScreen(d->clientToScreen(d->clientArea().pos())); // TODO: tidy, should be no call to DialogMgr

	POINT screenPosPoint = {(int)screenPos.x, (int)screenPos.y};
	ClientToScreen(AppBase().hWnd(), &screenPosPoint);
	RECT target = {(int)screenPos.x, (int)screenPos.y, (int)screenPos.x + desiredSize.x, (int)screenPos.y + desiredSize.y};
	/*AdjustWindowRectEx(&target, GetWindowLong(m_hWnd, GWL_STYLE), false, GetWindowLong(m_hWnd, GWL_EXSTYLE));*/

	setPos(Point2i(target.left, target.top));
	setSize(Point2i(target.right - target.left, target.bottom - target.top));

	// set dialog to 0,0 so it renders correctly
	m_uiBranch->addChild(m_dialog);
	m_uiBranch->setSize(m_dialog->size());
	m_dialog->setPos(Point2::ZERO);
}

Point2i OSWindow::pos() const
{
	RECT r;
	GetWindowRect(m_hWnd, &r);

	return Point2i(r.left, r.top);
}

Point2i OSWindow::size() const
{
	RECT r;
	GetWindowRect(m_hWnd, &r);

	return Point2i(r.right - r.left, r.bottom - r.top);
}

Point2i OSWindow::clientSize() const
{
	RECT r;
	GetClientRect(m_hWnd, &r);

	return Point2i(r.right - r.left, r.bottom - r.top);
}

void OSWindow::setPos(const Point2i& pos)
{
	Point2i s = size();
	MoveWindow(m_hWnd, pos.x, pos.y, s.x, s.y, true);
}

void OSWindow::setSize(const Point2i& size)
{
	SetWindowPos(m_hWnd, 0, 0, 0, size.x, size.y, SWP_NOMOVE);
	setupSwapChain();
}

WNDCLASSEX& OSWindow::sharedWndClass()
{
	if (m_sharedWndClass)
		return *m_sharedWndClass;
    
	std::string className = AppBase().name() + "_osmanaged";
	char* classBuf = new char[className.size()+1];
	strcpy(classBuf, className.c_str());

	m_sharedWndClass = new WNDCLASSEX;
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_PARENTDC, StaticMsgProc, 0L, sizeof(LONG), 
							GetModuleHandle(NULL), NULL, LoadCursor(0, IDC_ARROW), NULL, NULL,
							classBuf, NULL };
	*m_sharedWndClass = wc;
    RegisterClassEx( m_sharedWndClass );

	return *m_sharedWndClass;
}

// StaticMsgProc
LRESULT WINAPI OSWindow::StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	OSWindow* p = (OSWindow*)(DWORD_PTR)GetWindowLongPtr(hWnd, GWL_USERDATA);

	if (p)
		return p->MsgProc(hWnd, msg, wParam, lParam);
	else
		return DefWindowProc(hWnd, msg, wParam, lParam);
}

// MsgProc
LRESULT WINAPI OSWindow::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_dialog)
	{
		switch( msg )
		{
			case WM_CLOSE:
				revert();
				return 0;

			case WM_SIZE:
				{
					int width = LOWORD(lParam);
					int height = HIWORD(lParam);
					Point2 newSize(width / 640.0f, height / 640.0f);
					m_dialog->setSize(newSize);
					m_uiBranch->setSize(newSize);
					setupSwapChain();
					repaint();
					return 1;
				}

			case WM_SYSCOMMAND:
				if (wParam == 1)
				{
					revert();
					return 0;
				}
				break;

			case WM_SYSCHAR:
				// (alt-G)
				if (wParam == 'g')
					revert();
				return 0;

			case WM_ACTIVATE:
				if (LOWORD(wParam) != WA_INACTIVE)
				{
					DialogMgr().setActiveOSWindow(this);
				}
				return 0;
		}
	}

	HRESULT ret = Input().keyMsg(msg, wParam, lParam);
	if (ret != -1)
		return ret;

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void OSWindow::setupSwapChain()
{
	if (m_chain)
		m_chain->Release();

	Point2i s = Point2i(clientSize().x, clientSize().y);
	if (s.x == 0 || s.y == 0)
		return;

	IDirect3DSwapChain9* c;
	D3DD().GetSwapChain(0, &c);
	
	D3DPRESENT_PARAMETERS p;
	c->GetPresentParameters(&p);

	p.BackBufferWidth = clientSize().x;
	p.BackBufferHeight = clientSize().y;
	p.hDeviceWindow = m_hWnd;
    if (D3DD().CreateAdditionalSwapChain(&p, &m_chain) != D3D_OK)
	{
		revert();
		return;
	}

	Point2 scaleFactor = Point2(1.0f + m_dialog->size().x, 1.0f + m_dialog->size().y);

	m_projection = SDialogMgr::makeProjectionMatrix(Point2::ZERO, m_dialog->size());
}

void OSWindow::beginDraw()
{
	if (!m_dialog || !m_chain)
	{
		revert();
		return;
	}

	m_chain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_surface);

	D3DD().GetRenderTarget(0, &m_oldSurface);
	D3DD().SetRenderTarget(0, m_surface);
	D3DD().Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,255), 1.0f, 0 );

	D3DD().GetTransform(D3DTS_PROJECTION, &m_oldProjection);
	D3DD().SetTransform(D3DTS_PROJECTION, m_projection);
}

void OSWindow::endDraw()
{
	if (!m_dialog || !m_chain)
	{
		revert();
		return;
	}

	DialogMgr().drawPointer(m_uiBranch);

	D3DD().SetRenderTarget(0, m_oldSurface);

	m_chain->Present(0, 0, 0, 0, 0);

	D3DD().SetTransform(D3DTS_PROJECTION, &m_oldProjection);

	m_oldSurface->Release();
	m_surface->Release();
}

void OSWindow::revert()
{
	Dialog* d = m_dialog.ptr();
	d->setOSManaged(false);
}

void OSWindow::ensureInScreen()
{
	Point2i p = pos();
	Point2i s = size();
	
	Point2i screen(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	if (p.x + s.x > screen.x)
		p.x = screen.x - s.x;
	if (p.x < 0)
		p.x = screen.x;
	if (p.y + s.y > screen.y)
		p.y = screen.y - s.y;
	if (p.y < 0)
		p.y = screen.y;

	setPos(p);
}

void OSWindow::show(bool b)
{
	if (b)
		ShowWindow(m_hWnd, SW_SHOW);
	else
		ShowWindow(m_hWnd, SW_HIDE);
}

void OSWindow::repaint()
{
	if (m_uiBranch && m_dialog)
	{
		m_uiBranch->draw();
	}
}

PtrGC<class UIBranch> OSWindow::uiBranch() const 
{ 
	return m_uiBranch; 
}
