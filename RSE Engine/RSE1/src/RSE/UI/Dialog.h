// ---------------------------------------------------------------------------------------------------------
// 
// Dialog
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Controls/PlaceableControl.h"

class DialogFrame;
class UIMenu;


class RSE_API Dialog : public UIEditable
{
	USE_RTTI(Dialog, UIEditable);
	USE_EDITSTREAM;

public:
	Dialog();
	virtual ~Dialog();

	UISTYLE
		std::string controlsStyle;
	UISTYLEEND

	virtual std::string styleNameForControl(const RTTI& controlRTTI);

	virtual void setVisible(bool b);

	// convert to an OS-managed window
	virtual void setOSManaged(bool b);
	bool osManaged() { return m_osWindow != 0; }
	class OSWindow* osWindow() const { return m_osWindow; }

	// call this to close the dialog
	virtual void close();

	virtual void update(float delta);
	virtual void draw();
	virtual void drawByOwner(const PtrGC<UIElement>& owner);
	virtual void activate();
	virtual bool onPreDraw() { return false; };

	// adds a child to the client area
	virtual UIElement* _addChild(const PtrGC<UIElement>& p);

	// adds a child to the dialog itself
	virtual UIElement* addChildExplicit(const PtrGC<UIElement>& p);

	virtual void removeChild(const PtrGC<UIElement>& p);

	virtual bool usesFocus() const { return true; }

	virtual const DialogFrame& frame() const;
	virtual void setFrame(const RTTI& newFrameType);

	virtual UIElement& clientArea();
	virtual const UIElement& clientArea() const;

	const Point2& clientSize() const { return clientArea().size(); }
	void sizeClientToChildren();

	bool canUserClose() const { return m_bCanUserClose; }
	void setCanUserClose(bool b);

	const std::string title() const { if (!m_title.empty()) return m_title; return m_name; }
	void setTitle(const std::string& s);

	virtual void setName(const std::string& name);

	// input
	bool mouseDown(const Point2 &p, int button);
	bool mousePressed(const Point2 &p, int button);
	bool mouseUp(const Point2 &p, int button);

	// convenience
	virtual void setSizeForClient(const Point2& s);

	// events
	virtual void onFullscreenToggle(bool bFullscreen);

	virtual void saveUserSettings();
	virtual void restoreUserSettings();

	virtual void setPointer(PtrD3D<IDirect3DTexture9> pTex);
	virtual void setUsePointer(bool b);

	// editable
	virtual void editableProperties(PropertyEditorList& l);
	virtual void onEditableCommit();

	// context menu
	virtual void setContextMenu(const PtrGC<UIMenu>& menu);
	virtual const PtrGC<UIMenu>& contextMenu() const;

protected:
	virtual void construct();
	virtual void onPostRead();

	// context menu
	virtual void fillContextMenu(UIMenu& menu);

	virtual bool userSettingsShouldSave() const;
	virtual void userSettings(StreamVars& v);

	virtual void onActivate() {};
	virtual void onClose() {};
	virtual bool onMouseMove(const Point2 &p, const Point2 &delta) { return true; }
	virtual bool onMouseDown(const Point2 &p, int button) { return true; }
	virtual bool onMouseUp(const Point2 &p, int button) { return true; }

	void methodCloseDialog();	// shortcut for delegating

	PtrGC<UIElement>			m_clientArea;
	PtrGC<class DialogFrame>	m_frame;
	class OSWindow*				m_osWindow;
	bool						m_bIsOSWindow;
	Point2i						m_osWindowPos; // TODO: could encapsulate os/game state
	Point2						m_osWindowSize;
	Point2						m_gameSize;
	Point2						m_gamePos;
	bool						m_firstUpdate;

private:
	bool						m_bCanUserClose;
	std::string					m_title;
	std::string					m_frameRTTIName;
	PtrGC<UIMenu>				m_contextMenu;
};