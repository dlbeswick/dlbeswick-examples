#include "Standard/pch.h"
#include "AsyncEvent.h"

////

AsyncWaitBlock::AsyncWaitBlock(AsyncWaitable& event) :
	m_event(event)
{
	// wait until event is signalled (repeat if an APC interrupt)
	while (!m_event.wait()) {}
}

AsyncWaitBlock::~AsyncWaitBlock()
{
	m_event.onWaitEnded();
}

////

AsyncExclusiveBlock::AsyncExclusiveBlock(AsyncWaitable& event) :
	m_event(event)
{
	m_event.onExclusiveEnter();
	// wait until event is signalled (repeat if an APC interrupt), then reset so no-one else can access the block.
	while (!m_event.wait()) {}
}

AsyncExclusiveBlock::~AsyncExclusiveBlock()
{
	// this should signal the event
	m_event.onExclusiveExit();
}

////

AsyncEvent::AsyncEvent(bool signaled) :
	m_init(false),
	m_signaled(signaled)
{
	if (pthread_mutex_init(&m_mutex, 0) != 0)
		throwf("Mutex creation failed for AsyncEvent.");

	if (pthread_cond_init(&m_cond, 0) != 0)
		throwf("Condition variable creation failed for AsyncEvent.");
}

AsyncEvent::~AsyncEvent()
{
	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_cond);
}

void AsyncEvent::reset()
{
	pthread_mutex_lock(&m_mutex);
	m_signaled = false;
	pthread_mutex_unlock(&m_mutex);
}

void AsyncEvent::signal()
{
	pthread_mutex_lock(&m_mutex);
	m_signaled = true;
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

bool AsyncEvent::signaled()
{
	pthread_mutex_lock(&m_mutex);
	bool result = m_signaled;
	pthread_mutex_unlock(&m_mutex);

	return result;
}

bool AsyncEvent::wait(int ms)
{
	pthread_mutex_lock(&m_mutex);

	while (!m_signaled)
	{
		pthread_cond_wait(&m_cond, &m_mutex);
	}

	pthread_mutex_unlock(&m_mutex);

	return true;
}

void AsyncEvent::onWaitEnded()
{
	reset();
}

void AsyncEvent::onExclusiveEnter()
{
	// ensure that the handle will be created in the "signalled" state
	reset();
}

void AsyncEvent::onExclusiveExit()
{
	signal();
}


////
#if 0
AsyncEvents::AsyncEvents(int count, bool signaled) :
	m_count(count),
	m_activated(-1),
	m_signaled(new bool[count])
{
}

AsyncEvents::~AsyncEvents()
{
	delete[] m_signaled;
}

void AsyncEvents::signal(int i)
{
	AsyncEvent::
}

void AsyncEvents::reset(int i)
{
	ResetEvent(handle(i));
}

bool AsyncEvents::wait(bool all, int ms)
{
	int result = WaitForMultipleObjectsEx(m_count, &handle(0), all, ms, true);
	if (result == WAIT_IO_COMPLETION)
		return false;

	m_activated = result - WAIT_OBJECT_0;

	return true;
}

bool AsyncEvents::wait(int ms)
{
	return wait(false, ms);
}

void AsyncEvents::onWaitEnded()
{
	assert(m_activated != -1);
	reset(m_activated);
	m_activated = -1;
}

void AsyncEvents::signalAll()
{
	for (int i = 0; i < m_count; ++i)
	{
		signal(i);
	}
}

const HANDLE& AsyncEvents::handle(int i)
{
	if (!m_handles)
	{
		m_handles = new HANDLE[m_count];
		for (int i = 0; i < m_count; ++i)
		{
			m_handles[i] = CreateEvent(0, true, m_signaled, 0);
			if (!m_handles[i])
				throw ExceptionWinAPI();
		}
	}

	return m_handles[i];
}
#endif
