// ------------------------------------------------------------------------------------------------
//
// DlgProfile
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "DlgProfile.h"
#include "AppBase.h"
#include "Render/SDeviceD3D.h"
#include "UI/Controls/UIListView.h"
#include <Standard/Profiler.h>

REGISTER_RTTI_NAME(DlgProfile, "DlgProfile");

DlgProfile::DlgProfile() :
	m_nextUpdate(0)
{
	setSizeForClient(Point2(0.6f, 0.45f));
	align(ALIGN_CENTER, VALIGN_CENTER, parent());

	setFont("smallarial");

	m_pView = addChild(new UIListView);
	addChild(m_pView);
	m_pView->setSize(clientArea().size());
	m_pView->setColumns(3);
	m_pView->align(ALIGN_CENTER, VALIGN_CENTER, parent());

	m_pView->add(new ListText("Working Set"));
	m_pView->add(new ListText(""));
	m_pView->add(new ListText(""));
	m_pView->add(new ListText("Peak Working Set"));
	m_pView->add(new ListText(""));
	m_pView->add(new ListText(""));
	m_pView->add(new ListText("Avail. Texture"));
	m_pView->add(new ListText(""));
	m_pView->add(new ListText(""));
	m_variableStart = m_pView->numItems();

	// get memory info function if available
	m_pMemInfoFunc = 0;

	HMODULE hLib = LoadLibrary("psapi.dll");
	if (hLib)
	{
		m_pMemInfoFunc = (MemFunc)GetProcAddress(hLib, "GetProcessMemoryInfo");
	}

	setName("Performance Profiler");
}

void DlgProfile::update(float delta)
{
	Super::update(delta);

	if (AppBase().timer().time() > m_nextUpdate)
	{
		updateMemoryStats();
		m_nextUpdate = AppBase().timer().time() + 1.0;
	}

	updateStats();
}

void DlgProfile::updateMemoryStats()
{
	ListText* t;
	std::string s;

	if (m_pMemInfoFunc)
	{
		PROCESS_MEMORY_COUNTERS pmc;
		m_pMemInfoFunc(GetCurrentProcess(), &pmc, sizeof(pmc));

		// working set
		s = std::string("") + (int)pmc.WorkingSetSize;
		for (int i = (int)s.size() - 3; i >= 0; i -= 3)
			s.insert(i, 1, ',');
		t = (ListText*)m_pView->item(1);
		t->setText(s + " bytes");

		// peak working set
		s = std::string("") + (int)pmc.PeakWorkingSetSize;
		for (int i = (int)s.size() - 3; i >= 0; i -= 3)
			s.insert(i, 1, ',');
		t = (ListText*)m_pView->item(4);
		t->setText(s + " bytes");
	}

	// texture memory
	s = std::string("") + D3DD().GetAvailableTextureMem();
	for (int i = (int)s.size() - 3; i >= 0; i -= 3)
		s.insert(i, 1, ',');
	t = (ListText*)m_pView->item(7);
	t->setText(s + " bytes");
}

void DlgProfile::updateStats()
{
	uint idx = m_variableStart;

	const SProfiler::Records& r = Profiler().records();
	for (SProfiler::Records::const_iterator i = r.begin(); i != r.end(); ++i)
	{
		ListText* t;
		if (idx >= m_pView->numItems())
		{
			t = new ListText("");
			m_pView->add(t);
		}
		else
			t = m_pView->item(idx);

		t->setText(i->first);
		++idx;

		if (idx >= m_pView->numItems())
		{
			t = new ListText("");
			m_pView->add(t);
		}
		else
			t = m_pView->item(idx);

		if (i->second.accumulated)
			t->setText(ftostr((float)i->second.accumulated));
		else
			t->setText(ftostr((float)i->second.frameMS()));
		++idx;

		if (idx >= m_pView->numItems())
		{
			t = new ListText("");
			m_pView->add(t);
		}
		else
			t = m_pView->item(idx);

		t->setText(ftostr((float)i->second.totalMS));
		++idx;
	}
}