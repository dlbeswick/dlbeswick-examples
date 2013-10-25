// ------------------------------------------------------------------------------------------------
//
// DlgPhysTest
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "DlgStandardLevel.h"
#include "Game/Level.h"


class RSE_API DlgPhysTest : public DlgStandardLevel
{
	USE_RTTI(DlgPhysTest, DlgStandard);

public:
	DlgPhysTest();

	virtual void setupMenu(class UIMenu& menu);

protected:
	virtual void update(float delta);
	virtual void onDrawScene();
	virtual void onKeyChar(int key);
	
	virtual void reset();

private:
	bool m_bPlay;

	void onStep(const class MenuItemText&);
	void onStepSmall(const class MenuItemText&);
	void onReset(const class MenuItemText&) { reset(); }

	void onLoad(const class MenuItemText& item);
	void load(const std::string& terrainPath);
};