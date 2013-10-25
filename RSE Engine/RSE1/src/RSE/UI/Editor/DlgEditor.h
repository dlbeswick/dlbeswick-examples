// ---------------------------------------------------------------------------------------------------------
// 
// DlgEditor
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "UI/Dialogs/DlgStandard.h"
#include "UI/Controls/EditableProperties.h"
#include "UI/Controls/PlaceableControl.h"
#include "UIEditHandle.h"


class RSE_API DlgEditor : public DlgStandard
{
	USE_RTTI(DlgEditor, DlgStandard);

public:
	DlgEditor();

protected:
	virtual void fillContextMenu(UIMenu& menu);

	virtual void onDraw();
	virtual void onDrawScene() {}
	virtual void navigation() {}

	// update
	virtual void update(float delta);

	// input
	virtual bool onMouseDown(const Point2 &pos, int button);
	virtual bool onMousePressed(const Point2 &pos, int button);
	virtual bool onMouseUp(const Point2 &pos, int button);

	// methods
	void onAdd(const class MenuItemText& t);

	void showControlMenu(const Point2& screenPos);
	void onControlProperties(const class MenuItemText&);

	void setSelected(const PtrGC<UIElement>& e);
	void setSelected(const Point2& pos);

private:
	void updateHandles();
	void enableHandles(bool bEnabled);
	void onProperties(const MenuItemText&);
	void onSave(const MenuItemText&);
	void onLoad(const MenuItemText&);

	typedef std::map<std::string, PropertyEditorList> PropertiesMap;
	PropertiesMap m_properties;

	PtrGC<UIEditable>		m_pSelected;
	PtrGC<class DlgObjProperties>	m_dlgControlProps;

	PtrGC<UIEditHandle>		m_pHandles[8];
	Point2					m_clickPos;
	Point2					m_clickOffset;
	bool					m_bDragging;

	PtrGC<Dialog>			m_newDlg;
};
