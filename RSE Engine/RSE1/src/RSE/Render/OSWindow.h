// ------------------------------------------------------------------------------------------------
//
// OSWindow
// Contains the functionality required to create a new rendering viewport enclosed by an OS-handled
// window.
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_OSWINDOW_H
#define RSE_OSWINDOW_H

#include "RSE/RSE.h"
#include "Standard/Math.h"
#include "Standard/PtrGC.h"

class OSWindow
{
public:
	OSWindow(const PtrGC<class Dialog>& d, WNDCLASSEX& wndClass = sharedWndClass());
	~OSWindow();

	HWND hWnd() const { return m_hWnd; }
	virtual void setDialog(const PtrGC<class Dialog>& d);
	virtual void setPos(const Point2i& pos);
	virtual void setSize(const Point2i& size);

	virtual void beginDraw();
	virtual void endDraw();

	virtual void repaint();

	virtual void show(bool b);

	virtual Point2i pos() const;
	virtual Point2i size() const;
	virtual Point2i clientSize() const;
	virtual void ensureInScreen();
	PtrGC<class UIBranch> uiBranch() const;

	virtual void revert();

protected:
	virtual LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void setupSwapChain();
	static WNDCLASSEX& sharedWndClass();

	HWND						m_hWnd;
	WNDCLASSEX					m_wndClass;

	IDirect3DSurface9*			m_oldSurface;
	IDirect3DSurface9*			m_surface;
	IDirect3DSwapChain9*		m_chain;
	Matrix4						m_projection;
	D3DMATRIX					m_oldProjection;

	PtrGC<class Dialog>				m_dialog;
	PtrGC<class UIBranchOSWindow>	m_uiBranch;

private:
	static LRESULT WINAPI StaticMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	static WNDCLASSEX*		m_sharedWndClass;
};

#endif
