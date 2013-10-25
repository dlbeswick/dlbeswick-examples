// ------------------------------------------------------------------------------------------------
//
// DlgDebugPancake
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialogs/DlgDebugLevel.h"

class DlgGame;


class DlgDebugPancake : public DlgDebugLevel
{
public:
	DlgDebugPancake();

	virtual bool mouseDown(const Point2& pos, int button);

protected:
	virtual void construct();
	virtual void fillContextMenu(UIMenu& menu);
	virtual void onAIDebug(const PtrGC<class Man>& man);

	class DlgDebugAI* m_dlgDebugAI;
};