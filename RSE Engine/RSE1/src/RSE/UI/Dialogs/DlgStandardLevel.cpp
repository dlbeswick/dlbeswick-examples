// ---------------------------------------------------------------------------------------------------------
// 
// DlgStandardLevel
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "DlgStandardLevel.h"
#include "DlgDebugLevel.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/Scene.h"
#include "Render/SFont.h"
#include "Render/FontElement.h"
#include "Standard/Config.h"
#include "Standard/DXInput.h"
#include "Standard/ThreadStepper.h"
#include "UI/DialogMgr.h"
#include "DlgDebug.h"

IMPLEMENT_RTTI(DlgStandardLevel);

// constructor
DlgStandardLevel::DlgStandardLevel() :
	m_level(0),
	_pauseKey(VK_PAUSE),
	_pauseRequested(false)
{
	m_navigateRot = Point3(0,0,0);
}

DlgStandardLevel::~DlgStandardLevel()
{
	m_level.destroy();
}

void DlgStandardLevel::construct()
{
	Super::construct();
	setColour(RGBA(0,0,0,0));
}

// onPreDraw
bool DlgStandardLevel::onPreDraw()
{
	onDrawScene();
	return true;
}

// navigation
void DlgStandardLevel::navigation()
{
	if (!AppBase().options().get<bool>("Debug", "FreeCamera"))
		return;

	fpsNavigation();
}

void DlgStandardLevel::fpsNavigation()
{
	Camera& cam = level().scene().camera();
	Point3 pos = cam.pos();

 	float inc = navigationSpeed() * (float)level().time().frame();
	Point3 amt(0, 0, 0);
	if (focused())
	{
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
	}

	if (captured())
	{
		if (Input().mousePressed(0))
		{
			inc = navigationSpeed() * 0.005f;

			DialogMgr().pointerLock(true);

			m_navigateRot.z += inc * Input().mouseDeltaX();
			m_navigateRot.x += inc * Input().mouseDeltaY();
		}
		else
		{
			DialogMgr().pointerLock(false);
		}
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

void DlgStandardLevel::mouseNavigation()
{
	bool bMouseLeft = Input().mousePressed(0);
	bool bMouseRight = Input().key(VK_CONTROL);
	if (!captured())
	{
		if (!capturedElement())
		{
			setPointer(DialogMgr().stdTexture("pointer"));
			DialogMgr().pointerLock(false);
		}
		return;
	}

	DialogMgr().pointerLock(true);
	setPointer(0);

	Camera& cam = level().scene().camera();
	Point3 pos = cam.pos();

 	float inc = navigationSpeed() * (float)level().time().frame();
	Point3 amt(0, 0, 0);
	if (bMouseLeft && bMouseRight) // xz plane
	{
		amt.x = Input().mouseDeltaX() * inc;
		amt.z = -Input().mouseDeltaY() * inc;
	}
	else if (bMouseLeft) // xy plane
	{
		amt.x = Input().mouseDeltaX() * inc;
		amt.y = -Input().mouseDeltaY() * inc;
	}
	else if (bMouseRight) // rotation
	{
		m_navigateRot.z += Input().mouseDeltaX();
		m_navigateRot.x += Input().mouseDeltaY();
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

// showAxis
void DlgStandardLevel::showAxis()
{
	Font().set("worldarial");

	D3DPaint().setFill(RGBA(1, 0, 0, 1));
	D3DPaint().line(Point3(-10, 0, 0), Point3(10, 0, 0));
	D3DPaint().line(Point3(0, -10, 0), Point3(0, 10, 0));
	D3DPaint().line(Point3(0, 0, -10), Point3(0, 0, 10));
	D3DPaint().draw();

	font().worldWrite(level().scene().camera(), D3DPaint(), "x", Point3(10, 0, 0));
	font().worldWrite(level().scene().camera(), D3DPaint(), "y", Point3(0, 10, 0));
	font().worldWrite(level().scene().camera(), D3DPaint(), "z", Point3(0, 0, 10));
	D3DPaint().draw();
}

float DlgStandardLevel::navigationSpeed()
{
	return 1.0f;
}

Level& DlgStandardLevel::level()
{
	if (!m_level)
	{
		m_level = createLevel();
		AppBase().addLevel(*m_level);
	}

	return *m_level;
}

// update
void DlgStandardLevel::update(float delta)
{
	Super::update(delta);

	navigation();
}

// activate
void DlgStandardLevel::activate()
{
	enableDebug(true);
	dlgDebug().level = &level();
	enableDebug(false);

	Super::activate();
}

DlgDebug* DlgStandardLevel::createDlgDebug() const
{
	return new DlgDebugLevel;
}

void DlgStandardLevel::keyUp(int key)
{
	Super::keyUp(key);

	if (key == _pauseKey)
	{
		_pauseRequested = !_pauseRequested;
		
		// tbd: unify with debug menu shuttle code
		// note: the jerking on resume is caused by levels having their own timers. those timers should be
		// paused too, somehow.
		if (_pauseRequested)
		{
			AppBase().mainLoopStep().pause();
		}
		else
		{
			AppBase().mainLoopStep().resume();
		}
	}
}

bool DlgStandardLevel::pauseRequested()
{
	return _pauseRequested;
}
