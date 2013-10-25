// ---------------------------------------------------------------------------------------------------------
// 
// DlgStandard
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/Game/Level.h"
#include "RSE/UI/Dialog.h"
#include "RSE/UI/Controls/UIMenu.h"
#include "Standard/MultiDelegate.h"

class DlgDebug;


class RSE_API DlgStandard : public Dialog
{
	USE_RTTI(DlgStandard, Dialog);

public:
	DlgStandard();
	~DlgStandard();

	virtual void keyUp(int key);

	void setDebugKey(int key);

protected:
	virtual void construct();
	virtual void toggleDebug();
	virtual void enableDebug(bool bEnable);

	virtual void setupMenu(class UIMenu& menu) {};

	virtual DlgDebug* createDlgDebug() const;

	virtual void onDraw() {};

	int m_debugKey;

	DlgDebug& dlgDebug() const { return *m_dlgDebug; }

private:
	class DlgDebug* m_dlgDebug;
};
