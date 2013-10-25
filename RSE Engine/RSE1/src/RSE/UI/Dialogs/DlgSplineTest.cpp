// ---------------------------------------------------------------------------------------------------------
// 
// DlgSplineTest
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "DlgSplineTest.h"
#include "Render/D3DPainter.h"
#include "Render/FontElement.h"
#include "Render/SFont.h"
#include "UI/Controls/UIMenu.h"
#include "UI/DialogMgr.h"

//REGISTER_RTTI_NAME(DlgSplineTest, "Spline Test");
IMPLEMENT_RTTI(DlgSplineTest);


// constructor
DlgSplineTest::DlgSplineTest()
{
	m_path.add(Point3(-1, 10, 0));
	m_path.add(Point3(-0.5f, 10, 1));
	m_path.add(Point3(1, 10, 1));
	m_path.add(Point3(1, 10, -1));
	m_path.add(Point3(-1, 10, -1));
	m_state = Delegate();

	newState();
}

// setupMenu
void DlgSplineTest::setupMenu(UIMenu& menu)
{
	menu.addItem(new TMenuItemAssign<Delegate>("Reset", m_state, Delegate()));
	menu.addItem(new TMenuItemAssign<Delegate>("Trace", m_state, delegate(&DlgSplineTest::trace)));
	menu.addItem(new TMenuItemAssign<Delegate>("Arc Length Trace", m_state, delegate(&DlgSplineTest::arcLengthTrace)));
}

// onDrawScene
void DlgSplineTest::onDrawScene()
{
	// draw spline
	D3DPaint().setFill(RGBA(0, 1, 0));
	D3DPaint().splinePath(m_path);

	m_state();
}

// onDraw
void DlgSplineTest::onDraw()
{
	Font().set("arial");
	font().write(D3DPaint(), std::string("arc length: ") + m_path.arcLength(), 0, 0.75f - Font()->height());
	font().write(D3DPaint(), std::string("travelled length: ") + m_lastTravelLength, 0, 0.65f - Font()->height());
	D3DPaint().draw();
}

// update
void DlgSplineTest::update(float delta)
{
	Super::update(delta);

	m_delta = delta;
	m_time += delta;

	if (m_oldState != m_state)
	{
		newState();
	}

	m_oldState = m_state;
}

// newState
void DlgSplineTest::newState()
{
	m_time = 0;
	m_lastTravelLength = 0;
	m_lastPos = m_path.at(0);
}

// trace
void DlgSplineTest::trace()
{
	float t = m_time * 0.5f;
	if (t > 1)
		return;

	Point3 pos = m_path.eval(t);

	float size = 0.25;
	Quad q(
		Point3(-size, 0, size),
		Point3(size, 0, size),
		Point3(size, 0, -size),
		Point3(-size, 0, -size)
	);

	q.translate(pos);

	D3DPaint().setFill(RGBA(1, 0, 0, 1));
	D3DPaint().quad(q);

	m_lastTravelLength += (pos - m_lastPos).length();
	m_lastPos = pos;
}

// arcLengthTrace
void DlgSplineTest::arcLengthTrace()
{
	float t = m_time * 0.5f;
	if (t > 1)
		return;

	Point3 pos = m_path.evalArc(t);

	float size = 0.25;
	Quad q(
		Point3(-size, 0, size),
		Point3(size, 0, size),
		Point3(size, 0, -size),
		Point3(-size, 0, -size)
	);

	q.translate(pos);

	D3DPaint().setFill(RGBA(1,0,0));
	D3DPaint().quad(q);

	m_lastTravelLength += (pos - m_lastPos).length();
	m_lastPos = pos;
}
