// ---------------------------------------------------------------------------------------------------------
// 
// DlgEditor
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgEditor.h"
#include "PathResource.h"
#include "Standard/FileMgr.h"
#include "Standard/FileHelp.h"
#include "Standard/Exception/Filesystem.h"
#include "Render/SFont.h"
#include "UI/DialogMgr.h"
#include "UI/Controls/UIMenu.h"
#include "UI/Controls/UIButton.h"
#include "UI/Controls/PlaceableControl.h"
#include "UI/Dialogs/DlgDebug.h"
#include "UI/Dialogs/DlgMessageBox.h"
#include "UI/Dialogs/DlgObjProperties.h"
#include "UI/Controls/EditableProperties.h"

static const float DRAG_DELTA = 0.015f;

REGISTER_RTTI_NAME(DlgEditor, "Dialog Editor");


// constructor
DlgEditor::DlgEditor() :
	m_bDragging(false)
{
	m_newDlg = addChild(new Dialog);
	m_newDlg->setInputEnabled(false);

	Font().set("arial");

	Point2 scaling[8] = {Point2(0, -1), Point2(1, -1), Point2(1, 0), Point2(1, 1), Point2(0, 1),
							Point2(-1, 1), Point2(-1, 0), Point2(-1, -1)};

	// create handles
	for (uint i = 0; i < 8; ++i)
	{
		m_pHandles[i] = addChild(new UIEditHandle(delegate(&DlgEditor::updateHandles)));

		UIEditHandle& h = *m_pHandles[i];
		h.setVisible(false);
		h.setScaling(scaling[i]);
	}

	// disable debug context menu hook
	dlgDebug().setupMenu.clear();

	setContextMenu(new UIMenuText);
}


// update
void DlgEditor::update(float delta)
{
	Super::update(delta);
}


// onMouseDown
bool DlgEditor::onMouseDown(const Point2 &pos, int button)
{
	m_clickPos = pos;

	if (button == 1)
	{
		setSelected(pos);
	}
	else if (button == 0)
	{
		setSelected(pos);
		
		if (m_pSelected)
		{
			m_clickOffset = pos - m_pSelected->pos();
		}
	}

	return true;
}


// onMousePressed
bool DlgEditor::onMousePressed(const Point2 &pos, int button)
{
	if (button == 1 && dlgDebug().m_pMenu->visible())
	{
		dlgDebug().m_pMenu->setPos(pos);
	}
	else if (button == 0)
	{
		if (m_pSelected)
		{
			if (m_bDragging || (pos - m_clickPos).length() > DRAG_DELTA)
			{
				m_bDragging = true;
				m_pSelected->setPos(pos - m_clickOffset);
				captureMouse();
				updateHandles();
			}
		}
	}

	return true;
}


// onMouseUp
bool DlgEditor::onMouseUp(const Point2 &pos, int button)
{
	if (button == 1)
	{
		dlgDebug().m_pMenu->setInputEnabled(true);
	}
	else if (button == 0)
	{
		m_bDragging = false;
		if (captured())
			releaseMouse();
	}

	return false;
}


// onAdd
void DlgEditor::onAdd(const MenuItemText& t)
{
	UIElement* pNew = (UIElement*)PlaceableControl::newObject(t.data().c_str());
	if (!pNew)
		return;

	pNew->setPos(m_newDlg->screenToClient(contextMenu()->pos()));
	pNew->setInputEnabled(false);
	m_newDlg->addChild(pNew);

	setSelected(pNew);
}


// setSelected
void DlgEditor::setSelected(const Point2& pos)
{
	Point2 screen = clientToScreen(pos);

	// check for control selection
	for (ElementList::reverse_iterator i = m_newDlg->clientArea().children().rbegin(); i != m_newDlg->clientArea().children().rend(); ++i)
	{
		const PtrGC<UIElement>& e = *i;

		if (Cast<PlaceableControl>(e) && e->isPointIn(screen))
		{
			setSelected(e);
			return;
		}
	}

	// check for dialog selection
	if (m_newDlg->isPointIn(pos))
	{
		setSelected(m_newDlg);
		return;
	}
    
	setSelected(0);
}

void DlgEditor::setSelected(const PtrGC<UIElement>& e)
{
	m_pSelected = Cast<UIEditable>(e);

	updateHandles();
}


// updateHandles
void DlgEditor::updateHandles()
{
	for (uint i = 0; i < 8; i++)
	{
		m_pHandles[i]->setVisible(m_pSelected != 0);
	}

	// position edit handles
	if (m_pSelected)
	{
		Point2 pos = m_pSelected->screenPos();
		Point2 hsize = m_pSelected->size() * 0.5f;

		pos += hsize;

		for (uint i = 0; i < 8; i++)
		{
			UIEditHandle& h = *m_pHandles[i];

			Point2 newPos = pos + hsize.rComponentMul(h.scaling());
			newPos -= h.size() * 0.5f;
			
			h.setPos(newPos);
			h.setAffected(m_pSelected);
		}
	}
}


// onControlProperties
void DlgEditor::onControlProperties(const MenuItemText&)
{
	if (m_dlgControlProps)
	{
		// gross -- in the interest of reducing dependencies
		if (((DlgObjPropertiesPtrGC<UIElement>&)*m_dlgControlProps).obj() == Cast<PlaceableControl>(m_pSelected))
			return;

		removeChild(m_dlgControlProps);
	}

	m_dlgControlProps = addChild(new DlgObjPropertiesPtrGC<UIElement>(m_pSelected));
	m_dlgControlProps->setSize(Point2(0.5f, 0.35f));
	m_dlgControlProps->setPos(dlgDebug().m_pMenu->pos());
	m_dlgControlProps->setName("DlgEditor Control Properties");
	
	if (!m_pSelected->name().empty())
		m_dlgControlProps->setTitle(m_pSelected->rtti().className() + " - " + m_pSelected->name());
	else
		m_dlgControlProps->setTitle(m_pSelected->rtti().className());

	m_dlgControlProps->ensureInRect(m_dlgControlProps->screenPos(), m_dlgControlProps->size());
}

void DlgEditor::onDraw()
{
}

void DlgEditor::fillContextMenu(UIMenu& menu)
{
	// create controls
	PtrGC<UIMenu> pSub;

	// main menu
	menu.setTitle("Dialog Editor");
	
	// "Add" submenu
	pSub = menu.addExpander(new MenuItemText("Add"));
	RegistrantsList reg;
	registrantsOfClass(PlaceableControl::static_rtti(), reg);
	for (RegistrantsList::const_iterator i = reg.begin(); i != reg.end(); ++i)
	{
		pSub->addItem(new MenuItemText((*i)->name, (*i)->rtti.className(), delegate(&DlgEditor::onAdd)));
	}

	if (m_pSelected && Cast<UIEditable>(m_pSelected))
	{
		// show control menu
		menu.addItem(new MenuItemText("Properties...", delegate(&DlgEditor::onControlProperties)));
	}

	// Load
	menu.addItem(new MenuItemText("Load", delegate(&DlgEditor::onLoad)));

	// Save
	menu.addItem(new MenuItemText("Save", delegate(&DlgEditor::onSave)));
}

void DlgEditor::onSave(const MenuItemText&)
{
	try
	{
		if (m_newDlg->name().empty())
			throwf("Dialog has no name");

		Path("media\\dialogs").create();
		
		obinstream f(Path("media\\dialogs\\" + m_newDlg->name() + ".dlg").open(std::ios::out | std::ios::binary));
		f << *m_newDlg;
	}
	catch (const Exception& e)
	{
		addChild(new DlgMessageBox("Error saving dialog", e));
	}
}

void DlgEditor::onLoad(const MenuItemText&)
{
	try
	{
		ibinstream s(PathResource("media\\dialogs\\DlgFileBrowser.dlg").open(std::ios::in | std::ios::binary));

		UIElement* loaded = constructFromStream(s);
		if (!Cast<Dialog>(loaded))
			throwf("Not a valid dialog file (dialog object not found)");
		s >> *loaded;

		removeChild(m_newDlg);
		m_newDlg = Cast<Dialog>(loaded);
		addChild(m_newDlg);
	}
	catch (ExceptionFilesystem& e)
	{
		addChild(new DlgMessageBox("Error loading dialog", e.what()));
	}
}
