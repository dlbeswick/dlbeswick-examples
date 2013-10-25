#include "Standard/pch.h"
#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_obj);
	m_success = false;
}

void CriticalSection::enter()
{
	EnterCriticalSection(&m_obj);
	m_success = true;
}

bool CriticalSection::success() const
{
	return m_success;
}

bool CriticalSection::tryEnter()
{
	m_success = TryEnterCriticalSection(&m_obj) != 0;
	return m_success;
}

void CriticalSection::leave()
{
	LeaveCriticalSection(&m_obj);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_obj);
}
