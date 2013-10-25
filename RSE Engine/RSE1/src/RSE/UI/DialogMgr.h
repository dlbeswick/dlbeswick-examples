// ---------------------------------------------------------------------------------------------------------
// 
// DialogMgr
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_DIALOGMGR_H
#define RSE_DIALOGMGR_H

#include "Standard/PendingList.h"
#include "Standard/Singleton.h"
#include "RSE/UI/Dialog.h"

class Camera;
class UIBranch;

class RSE_API SDialogMgr : public Singleton<SDialogMgr>, public Base
{
public:
	SDialogMgr();
	~SDialogMgr();

	void create();
	void update(float delta);

	void beginDraw();
	void draw();

	Dialog* add(const std::string& id);
	void remove(const PtrGC<Dialog>& pDlg);
	void removeAll();

	// materials
	const Material& material(const std::string& name);

	// events
	void onFullscreenToggle(bool bFullscreen);
	void onMouseOverElement(UIElement* e);
	void onElementPointerChange(UIElement* e);

	// os window support
	void setActiveOSWindow(OSWindow* w);
	void onOSWindowDestroyed(OSWindow* w);

	// ui branches
	void addUIBranch(const PtrGC<UIBranch>& uiBranch);
	void setActiveBranch(const PtrGC<UIBranch>& uiBranch);
	PtrGC<UIElement> mainBranch() const;
	PtrGC<UIElement> branchFor(UIElement* e);

	// pointer
	float pointerSensitivity() const { return m_pointerSensitivity; }
	void setPointerSensitivity(float scale) { m_pointerSensitivity = scale; }
	Point2 gameToScreen(const Point2& units);
	Point2 screenToGame(Point2 units);
	void drawPointer(const PtrGC<UIElement>& uiBranch); // boo, make nicer
	uint doubleClickTime() const;

	const DMath::Point2& pointerPos() const { return m_pointerPos; }
	const DMath::Point2& pointerDelta() const { return m_pointerDelta; }
	void pointerLock(bool bLock);
	void pointerClamp(bool bClamp);

	HWND activeWindow() const;
	const PtrGC<UIElement>& activeBranch() const { return m_activeBranch; }

	Camera& camera() { return *m_camera; }

	PtrD3D<IDirect3DTexture9> stdTexture(const std::string& tex);

	static Matrix4 makeProjectionMatrix(Point2 origin, const Point2& size);

protected:

	void onHelpHover();

	typedef stdext::hash_map<std::string, PtrD3D<IDirect3DTexture9> > TextureList;
	TextureList m_stdTextures;

	PtrGC<UIElement>	m_root;
	PtrGC<class UIBranch>	m_mainBranch;
	PtrGC<UIElement>	m_activeBranch;
	class OSWindow* 	m_activeOSWindow;
	Camera* m_camera;

	typedef stdext::hash_map<std::string, Material*> Materials;
	Materials m_materials;

	PtrD3D<IDirect3DTexture9> m_currentPointerTex; // filled in dialog update
	Point2 m_pointerPos;
	Point2 m_pointerDelta;
	float m_pointerSensitivity;
	bool m_bPointerLock;
	bool m_bPointerClamp;
	uint m_doubleClickTime;
    Counter m_helpCounter;
	class RenderContext* m_context;
};

class RSE_API UIBranch : public UIContainer
{
	USE_RTTI(UIBranch, UIContainer);
};

inline SDialogMgr &DialogMgr() { return SDialogMgr::instance(); }

#endif
