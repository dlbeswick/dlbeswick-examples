// ---------------------------------------------------------------------------------------------------------
// 
// DlgStandardLevel
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgStandard.h"

class DlgDebugLevel;


class RSE_API DlgStandardLevel : public DlgStandard
{
	USE_RTTI(DlgStandardLevel, DlgStandard);

public:
	DlgStandardLevel();
	~DlgStandardLevel();

	virtual void activate();
	virtual void keyUp(int key);
	virtual void update(float delta);
	virtual bool onPreDraw();

	Level& level();
	virtual bool pauseRequested();

protected:
	virtual void construct();
	virtual void onDrawScene() { if (m_level) level().draw(); };

	virtual void navigation();
	virtual void fpsNavigation();
	virtual void mouseNavigation();
	virtual float navigationSpeed();

	void showAxis();

	virtual Level* createLevel() { return new Level; }

	virtual DlgDebug* createDlgDebug() const;
	DlgDebugLevel& dlgDebug() const { return (DlgDebugLevel&)Super::dlgDebug(); }

	int _pauseKey;
	bool _pauseRequested;

	Point3 m_navigateRot;

	PtrGC<Level> m_level;
};
