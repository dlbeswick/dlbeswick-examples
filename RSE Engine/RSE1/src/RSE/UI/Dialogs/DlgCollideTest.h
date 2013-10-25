// ---------------------------------------------------------------------------------------------------------
// 
// DlgCollideTest
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandardLevel.h"


class RSE_API DlgCollideTest : public DlgStandardLevel
{
	USE_RTTI(DlgCollideTest, DlgStandard);

public:
	DlgCollideTest();

	virtual void activate();
	virtual void setupMenu(class UIMenu& menu);

protected:

	virtual void navigation();

	virtual bool onMouseUp(const Point2& p, int button);

	virtual void update(float delta);

	void onCreate(const TMenuItemData<const Registrant*>& d);

	class PtrGC<class PhysicsObject> m_selected;
	class DragHelper3D* m_dragger;
};