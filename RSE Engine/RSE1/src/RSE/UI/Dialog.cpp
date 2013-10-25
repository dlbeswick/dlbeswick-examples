// ---------------------------------------------------------------------------------------------------------
// 
// Dialog
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "Dialog.h"
#include "DialogFrame.h"
#include "DialogMgr.h"
#include "UI/Controls/UIMenu.h"
#include "UI/Controls/UIPropertyEditor.h"
#include "Render/D3DPainter.h"
#include <Render/FontElement.h>
#include "Render/SDeviceD3D.h"
#include "Render/SFont.h"
#include "Render/OSWindow.h"
#include "Standard/DXInput.h"

IMPLEMENT_RTTI(DialogClientArea);

#define METADATA Dialog
REGISTER_RTTI(Dialog);

// streaming
EDITSTREAM
	EDITSTREAMVAR(m_size, "");
	EDITSTREAMVAR(m_pos, "");
	EDITSTREAMVAR(m_title, "");
	EDITSTREAMVAR(m_bCanUserClose, "If false, a close button can not be shown for the dialog.");
}

UISTYLESTREAM(Dialog)
	STREAMVAR(controlsStyle);
UISTYLESTREAMEND

void Dialog::editableProperties(PropertyEditorList& l)
{
	Super::editableProperties(l);

	// frames
	std::vector<std::string> frames;
	registrantNamesOfClass(DialogFrame::static_rtti(), frames);

	// fix -- old style code
	l.push_back(property("Frame Type", frames, m_frameRTTIName));
}

void Dialog::onEditableCommit()
{
	setFrame(registrant(m_frameRTTIName)->rtti);
	setCanUserClose(m_bCanUserClose);
}

// constructor
Dialog::Dialog() :
	m_osWindow(0),
	m_bIsOSWindow(0),
	m_bCanUserClose(true),
	m_osWindowPos(INT_MAX, INT_MAX),
	m_osWindowSize(FLT_MAX, FLT_MAX),
	m_firstUpdate(true)
{
	setConfig(AppBase().ui());
}

void Dialog::construct()
{
	Super::construct();

	m_clientArea = new DialogClientArea; // frame adds it as a child, but it's re-used between frames.

	setPointer(DialogMgr().stdTexture("pointer"));
	setUsePointer(true);

	setFrame(DialogFrameOverlapped::static_rtti());

	if (parent())
	{
		setSize(parent()->size());
		clientArea().setSize(size());
	}
	else
	{
		setSize(DialogMgr().activeBranch()->size());
		clientArea().setSize(size());
	}

	setColour(RGBA(0,0,1));
}

// destructor
Dialog::~Dialog()
{
	// save this var so user settings are saved correctly
	bool osManaged = m_bIsOSWindow;

	if (m_osWindow)
		setOSManaged(false);

	m_bIsOSWindow = osManaged;

	saveUserSettings();
}

// update
void Dialog::update(float delta)
{
	UIElement::update(delta);

	if (m_firstUpdate)
		activate();

	if (visible() && !focusedElement())
	{
		PtrGC<UIElement> newFocus = clientArea().findFocus();
		if (newFocus)
			newFocus->setFocus(); 
	}

	m_firstUpdate = false;
}


// draw
void Dialog::draw()
{
	Super::draw();
}

void Dialog::methodCloseDialog()
{
	close();
}

UIElement* Dialog::_addChild(const PtrGC<UIElement>& p)
{
	// add to client area by default
	return clientArea().addChild(p);
}

UIElement* Dialog::addChildExplicit(const PtrGC<UIElement>& p)
{
	return Super::_addChild(p);
}

void Dialog::removeChild(const PtrGC<UIElement>& p)
{
	// remove from client area by default
	clientArea().removeChild(p);
}

void Dialog::setFrame(const RTTI& newFrameType)
{
	if (m_frame && &m_frame->rtti() == &newFrameType)
		return;

	DialogFrame* newFrame = Cast<DialogFrame>(Registrar<Base>::newObject(newFrameType, this));
	if (!newFrame)
	{
		newFrame = new DialogFrameNaked(this);
	}

	addChildExplicit(newFrame);

	if (m_frame)
	{
		m_frame.destroy();
	}

	m_frame = newFrame;
	m_frame->onDialogStatusChange();

	m_frameRTTIName = m_frame->rtti().className();
}

void Dialog::setSizeForClient(const Point2& s)
{
	setSize(m_frame->sizeNeededForClient(s));

	clientArea().setSize(s);
}

void Dialog::sizeClientToChildren()
{
	if (clientArea().children().empty())
		return;

	Point2 min(FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX);

	for (Dialog::ElementList::iterator i = clientArea().children().begin(); i != clientArea().children().end(); ++i)
	{
		UIElement& e = **i;
		min.x = std::min(e.pos().x, min.x);
		min.y = std::min(e.pos().y, min.y);
		max.x = std::min(e.pos().x + e.size().x, max.x);
		max.y = std::min(e.pos().y + e.size().y, max.y);
	}

	clientArea().setSize(max - min);
}

void Dialog::close()
{
	onClose();
	PtrGC<Dialog>(this).destroy();
}

void Dialog::setOSManaged(bool b)
{
	if (!D3D().isWindowed() && b)
		return;

	m_bIsOSWindow = b;

	if (b)
	{
		if (!m_osWindow)
		{
			m_gameSize = size();
			m_gamePos = pos();

			if (m_osWindowSize.x != FLT_MAX && m_osWindowSize != Point2::ZERO)
				setSize(m_osWindowSize);

			m_osWindow = new OSWindow(this);

			// reposition windows mouse cursor to correspond to game screen position
			Point2 pos = DialogMgr().pointerPos();
			pos.x *= D3D().screenSize().x;
			pos.y *= D3D().screenSize().y * 1.3333f;
			POINT winPoint;
			winPoint.x = (int)pos.x;
			winPoint.y = (int)pos.y;
			ClientToScreen(AppBase().hWnd(), &winPoint);
			SetCursorPos(winPoint.x, winPoint.y);

			if (m_osWindowPos.x != INT_MAX)
				m_osWindow->setPos(m_osWindowPos);
			m_osWindow->ensureInScreen();
			m_osWindow->show(true);
		}
	}
	else
	{
		if (m_osWindow)
		{
			m_osWindowPos = m_osWindow->pos();
			m_osWindowSize = size();
			delete m_osWindow;
			m_osWindow = 0;
			setSize(m_gameSize);
			
			// fix: on shutdown, dialogmgr is invalid
			// possible fix -- check App().running()
			//ensureInRect(m_gamePos, size());
		}
	}
}

void Dialog::setCanUserClose(bool b)
{
	m_bCanUserClose = b;
	if (m_frame)
		m_frame->onDialogStatusChange();
}

void Dialog::setName(const std::string& name)
{
	Super::setName(name);
	setConfigCategory(m_name);
	restoreUserSettings();
}

void Dialog::setTitle(const std::string& s)
{
	m_title = s;
}

void Dialog::saveUserSettings()
{
	if (!userSettingsShouldSave() || configCategory().empty())
		return;

	StreamVars v;
	userSettings(v);
	Streamable::write(config(), configCategory(), v);
}

bool Dialog::userSettingsShouldSave() const
{
	return !m_title.empty();
}

void Dialog::restoreUserSettings()
{
	if (configCategory().empty())
		return;

	StreamVars v;
	userSettings(v);
	Streamable::read(config(), configCategory(), v);
}

void Dialog::userSettings(StreamVars& v)
{
	STREAMVAR(m_size);
	STREAMVAR(m_pos);
	STREAMVAR(m_bIsOSWindow);
	STREAMVAR(m_osWindowPos);
	STREAMVAR(m_osWindowSize);
}

void Dialog::onPostRead()
{
	if (!m_frameRTTIName.empty())
	{
		const Registrant* r = Base::registrant(m_frameRTTIName.c_str());
		if (r)
			setFrame(r->rtti);
		else
			setFrame(DialogFrameOverlapped::static_rtti());
	}

	// tbd: not working, may cause crash
	/*if (m_bIsOSWindow && !m_osWindow)
	{
		setOSManaged(true);
		m_osWindow->setPos(m_osWindowPos);
	}*/
}

void Dialog::onFullscreenToggle(bool bFullscreen)
{
	if (m_osWindow && bFullscreen)
		m_osWindow->revert();
}

void Dialog::setPointer(PtrD3D<IDirect3DTexture9> pTex)
{
	Super::setPointer(pTex);

	clientArea().setPointer(pTex);
}

void Dialog::setUsePointer(bool b)
{
	Super::setUsePointer(b);
	
	clientArea().setUsePointer(b);
}

void Dialog::setVisible(bool b)
{
	bool wasVisible = visible();

	Super::setVisible(b);

	// calculate focus when becoming visible
	if (!wasVisible && b)
	{
		PtrGC<UIElement> newFocus = clientArea().findFocus();
		if (newFocus)
			newFocus->setFocus(); 
	}
}

void Dialog::setContextMenu(const PtrGC<UIMenu>& menu)
{
	m_contextMenu = menu;
	addChild(m_contextMenu);
}

const PtrGC<UIMenu>& Dialog::contextMenu() const
{
	return m_contextMenu;
}

void Dialog::fillContextMenu(UIMenu& menu)
{
	menu.setTitle("Context Menu");
	menu.addItem(new MenuItemText("Hello."));
}

bool Dialog::mouseUp(const Point2& p, int button)
{
	if (m_contextMenu)
	{
		m_contextMenu->setInputEnabled(true);
	}

	releaseMouse();

	return Super::mouseUp(p, button);
}

bool Dialog::mousePressed(const Point2& p, int button)
{
	// handle context menu
	if (m_contextMenu)
	{
		if (m_contextMenu->visible() && button == 1)
		{
			m_contextMenu->clear();
			fillContextMenu(*m_contextMenu);
			m_contextMenu->setPos(p - Point2(0, m_contextMenu->font().height()));
			return true;
		}
	}

	return Super::mousePressed(p, button);
}

bool Dialog::mouseDown(const Point2& p, int button)
{
	if (button == 1)
	{
		if (m_contextMenu)
		{
			captureMouse();
			m_contextMenu->setVisible(true);
			m_contextMenu->setInputEnabled(false);
			m_contextMenu->bringToFront();
			bringToFront();

			return true;
		}
	}
	else
	{
		if (m_contextMenu)
			m_contextMenu->setVisible(false);
	}

	return Super::mouseDown(p, button);
}

void Dialog::activate()
{
	PtrGC<UIElement> newFocus = clientArea().findFocus();
	if (newFocus)
		newFocus->setFocus(); 
	onActivate();
};

void Dialog::drawByOwner(const PtrGC<UIElement>& owner)
{
	onPreDraw();
	D3DPaint().reset();
	Super::drawByOwner(owner);
}

std::string Dialog::styleNameForControl(const RTTI& controlRTTI)
{
	return configUIStyles().get<std::string>(style().controlsStyle, controlRTTI.className());
}

UIElement& Dialog::clientArea()
{ 
	// You should probably be doing whatever it was you were doing in 'construct' rather than the constructor.
	// Either that, or you've forgotten to call Super::construct().
	// Or, you haven't set up RTTI properly for the class.
	assert(m_clientArea);

	return *m_clientArea; 
}

const UIElement& Dialog::clientArea() const
{ 
	return *m_clientArea; 
}

const DialogFrame& Dialog::frame() const 
{ 
	return *m_frame; 
}
