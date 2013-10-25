#include "Standard/pch.h"
#include "CriticalSection.h"
#include <Foundation/Foundation.h>


CriticalSection::CriticalSection()
{
	m_obj = [[NSRecursiveLock alloc] init];
	m_success = false;
}

void CriticalSection::enter()
{
	[m_obj lock];
	m_success = true;
}

bool CriticalSection::success() const
{
	return m_success;
}

bool CriticalSection::tryEnter()
{
	[m_obj tryLock];
	return m_success;
}

void CriticalSection::leave()
{
	[m_obj unlock];
}

CriticalSection::~CriticalSection()
{
	[m_obj release];
}

