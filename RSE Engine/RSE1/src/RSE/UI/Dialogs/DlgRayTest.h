// ---------------------------------------------------------------------------------------------------------
// 
// DlgRayTest
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandardLevel.h"


class RSE_API DlgRayTest : public DlgStandardLevel
{
	USE_RTTI(DlgRayTest, DlgStandardLevel);

public:
	DlgRayTest();

	virtual void setupMenu(class UIMenu& menu);

	struct State
	{
		virtual void draw(Level& level) = 0;
		virtual void update(Level& level, float delta) = 0;
		virtual Point3 camPos() { return Point3(0, 0, 0); }
		virtual Quat camRot() { return Quat(); }
	};

protected:

	SmartPtr<State> m_state;

	virtual void onDrawScene();
	virtual void update(float delta);
	virtual void onStart();
};