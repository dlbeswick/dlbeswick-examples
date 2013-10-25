// ---------------------------------------------------------------------------------------------------------
// 
// DlgAnimTest
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandardLevel.h"
#include "Render/SkelMesh.h"
#include "UI/Controls/UIMenu.h"


class RSE_API DlgAnimTest : public DlgStandardLevel
{
	USE_RTTI(DlgAnimTest, DlgStandardLevel);

public:
	DlgAnimTest();

protected:
	virtual void setupMenu(class UIMenu& menu);

	virtual void onDrawScene();
	virtual void onActivate();
	virtual void update(float delta);
	virtual void onDraw();
	virtual void onFrame(const class MenuItemText&);

	void onRegression(const class MenuItemText&);
	void onRewind(const class MenuItemText&);

	SkelMesh m_mesh;
	PtrGC<UIMenuText> m_regressionMenu;
	bool m_bStopped;
	bool m_bLooping;
};