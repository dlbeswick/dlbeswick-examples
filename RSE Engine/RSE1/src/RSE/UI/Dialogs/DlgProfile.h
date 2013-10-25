// ------------------------------------------------------------------------------------------------
//
// DlgProfile
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "UI/Dialog.h"
#include "UI/Controls/UIListView.h"


class DlgProfile : public Dialog
{
	USE_RTTI(DlgProfile, Dialog);

public:
	DlgProfile();

protected:
	virtual void update(float delta);

	void updateMemoryStats();
	void updateStats();

	UIListView* m_pView;
	
	typedef BOOL (CALLBACK* MemFunc)(HANDLE, PROCESS_MEMORY_COUNTERS*, DWORD);
	MemFunc m_pMemInfoFunc;
	double	m_nextUpdate;
	int m_variableStart;
};