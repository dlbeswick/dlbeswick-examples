#include "Standard/pch.h"
#include "AsyncEvent.h"
#include "Standard/CriticalSectionBlock.h"
#include "ExceptionWinAPI.h"

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
	m_handle(0),
	m_signaled(signaled)
{
	handle(); // temp -- lazy evaluation is not yet threadsafe
}

HANDLE AsyncEvent::handle()
{
	if (!m_handle)
		m_handle = CreateEvent(0, true, m_signaled, 0);
	if (!m_handle)
		throw ExceptionWinAPI();

	return m_handle;
}

AsyncEvent::~AsyncEvent()
{
	CloseHandle(m_handle);
}

bool AsyncEvent::signaled()
{
	{
		Critical(m_signaledMutex);
		return m_signaled;
	}
}

void AsyncEvent::reset()
{
	{
		Critical(m_signaledMutex);
		m_signaled = false;
	}

	ResetEvent(handle());
}

void AsyncEvent::signal()
{
	{
		Critical(m_signaledMutex);
		m_signaled = true;
	}

	SetEvent(handle());
}

bool AsyncEvent::wait(int ms)
{
	return WaitForSingleObjectEx(handle(), ms, true) != WAIT_IO_COMPLETION;
}

void AsyncEvent::onWaitEnded()
{
	reset();
}

void AsyncEvent::onExclusiveEnter()
{
	Critical(m_signaledMutex);
	// ensure that the handle will be created in the "signalled" state
	m_signaled = true;
}

void AsyncEvent::onExclusiveExit()
{
	signal();
}


////

AsyncEvents::AsyncEvents(int count, bool signaled) :
	m_count(count),
	m_activated(-1),
	m_handles(0),
	m_signaled(signaled)
{
}

AsyncEvents::~AsyncEvents()
{
	if (m_handles)
	{
		for (int i = 0; i < m_count; ++i)
		{
			CloseHandle(m_handles[i]);
		}
		delete[] m_handles;
	}
}

void AsyncEvents::signal(int i)
{
	SetEvent(handle(i));
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
