#include "Standard/pch.h"
#include "CriticalSection.h"
#include <pthread.h>

CriticalSection::CriticalSection() :
	 m_success(false)
{
	pthread_mutexattr_init(&_attr);
	pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_mutex, &_attr);
}

CriticalSection::~CriticalSection()
{
	pthread_mutex_destroy(&_mutex);
	pthread_mutexattr_destroy(&_attr);
}

void CriticalSection::enter()
{
	pthread_mutex_lock(&_mutex);
	m_success = true;
}

bool CriticalSection::success() const
{
	return m_success;
}

bool CriticalSection::tryEnter()
{
	if (pthread_mutex_trylock(&_mutex) == 0)
	{
		m_success = true;
		return true;
	}

	return false;
}

void CriticalSection::leave()
{
	pthread_mutex_unlock(&_mutex);
	m_success = false;
}
