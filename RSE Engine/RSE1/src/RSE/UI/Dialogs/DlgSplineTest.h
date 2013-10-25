// ---------------------------------------------------------------------------------------------------------
// 
// DlgSplineTest
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandard.h"
#include "Standard/Spline.h"

class RSE_API DlgSplineTest : public DlgStandard
{
	USE_RTTI(DlgSplineTest, DlgStandard);

public:
	DlgSplineTest();

protected:
	virtual void setupMenu(class UIMenu& menu);
	virtual void onDrawScene();
	virtual void onDraw();
	virtual void update(float delta);

	void newState();
	void trace();
	void arcLengthTrace();

	Delegate m_state;
	Delegate m_oldState;
	Delegate m_inputState;
	
//	virtual void navigation() {};

	CatmullRomPath m_path;

	float m_delta;
	float m_time;
	float m_lastTravelLength;
	Point3 m_lastPos;
};