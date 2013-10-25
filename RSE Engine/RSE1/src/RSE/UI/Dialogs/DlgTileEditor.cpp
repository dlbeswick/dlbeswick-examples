// ------------------------------------------------------------------------------------------------
//
// DlgTileEditor
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "DlgTileEditor.h"
#include "AppBase.h"
#include "PathResource.h"
#include "Game/ObjectQuadtree.h"
#include "Render/MeshObject.h"
#include "UI/DialogMgr.h"
#include "UI/Controls/UIScrollBar.h"
#include "UI/Controls/UIListView.h"
#include "Standard/DXInput.h"
#include "Standard/FileMgr.h"

//REGISTER_RTTI_NAME(DlgTileEditor, "Tile-Based Editor");
IMPLEMENT_RTTI(DlgTileEditor);

// construct
DlgTileEditor::DlgTileEditor()
{
	PtrGC<MeshObject> o;
	o = new MeshObject;
	o->setParent(level());

	itextstream roadtest(PathResource("media/level/roadtest.!x").open());
	o->load(roadtest);
}

void DlgTileEditor::navigation()
{
	Camera& cam = level().scene().camera();
	Point3 pos = cam.pos();

 	float inc = 200.0f * AppBase().freq();
	Point3 amt(0, 0, 0);
	if (Input().key('A'))
	{
		amt.x -= inc;
	}
	if (Input().key('D'))
	{
		amt.x += inc;
	}
	if (Input().key('W'))
	{
		amt.y += inc;
	}
	if (Input().key('S'))
	{
		amt.y -= inc;
	}

	if (Input().mousePressed(0))
	{
		inc = 2;

		DialogMgr().pointerLock(true);

		m_navigateRot.z += inc * Input().mouseDeltaX();
		m_navigateRot.x += inc * Input().mouseDeltaY();
	}
	else
	{
		DialogMgr().pointerLock(false);
	}

	Quat q;
	Quat q2;

	q.angleAxis(m_navigateRot.z, Point3(0, 0, -1));
	q2.angleAxis(m_navigateRot.x, Point3(-1, 0, 0));
	q.multiply(q2);

	q.normalise();
	cam.setRot(q);
	pos += amt * q;
	cam.setPos(pos);
}
