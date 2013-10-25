// ------------------------------------------------------------------------------------------------
//
// DlgDebugAI
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/UI/Dialog.h"


class DlgDebugAI : public Dialog
{
public:
	USE_RTTI(DlgDebugAI, Dialog);

	DlgDebugAI();

	void set(const PtrGC<class ManAIControl>& ai);

	virtual void update(float delta);

protected:
	virtual void construct();

	PtrGC<class ManAIControl> m_ai;
	class UIListView* m_info;
};