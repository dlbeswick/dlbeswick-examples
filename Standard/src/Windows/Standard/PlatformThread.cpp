// ------------------------------------------------------------------------------------------------
//
// Thread
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "AsyncEvent.h"
#include "Standard/Thread.h"
#include "Standard/Log.h"
#include "ExceptionWinAPI.h"

VOID CALLBACK PlatformThread::stopAPCProc(ULONG_PTR dwParam)
{
	PlatformThread* self = (PlatformThread*)dwParam;
	ddbg << "Thread " << itostr(self->m_id) << " aborted." << ddbg.endl;
	if (!self->m_finishedEvent->signaled())
		throw(ExceptionThreadAbort());
}

VOID CALLBACK PlatformThread::softSuspendAPCProc(ULONG_PTR dwParam)
{
	PlatformThread* self = (PlatformThread*)dwParam;
//	ddbg << "Thread " << itostr(self->m_id) << " under soft suspend." << ddbg.endl;
	self->m_softSuspendReadyEvent->signal();
	self->m_softSuspendResumeEvent->wait();
//	ddbg << "Thread " << itostr(self->m_id) << " resumed from soft suspend." << ddbg.endl;
}

PlatformThread::PlatformThread(bool suspended) :
	m_finished(true),
	m_aborting(false),
	m_suspended(false),
	m_softSuspended(false),
	m_softSuspendResumeEvent(new AsyncEvent),
	m_softSuspendReadyEvent(new AsyncEvent),
	m_finishedEvent(new AsyncEvent),
	ThreadBase(suspended)
{
}

PlatformThread::~PlatformThread()
{
	// ensure we signal to anyone waiting for a finish from us, if no user APC has intervened in the
	// meantime.
	m_finishedEvent->signal(); 

	delete m_softSuspendResumeEvent;
	delete m_softSuspendReadyEvent;
	delete m_finishedEvent;
}

void PlatformThread::platformStop()
{
	// tbd: accessing and setting m_finished and m_aborting should be protected with mutexes.
	if (m_finished || m_aborting)
		return;

	m_aborting = true;
	ddbg << "Thread " << itostr(m_id) << " is aborting." << ddbg.endl;
	QueueUserAPC(stopAPCProc, m_handle, (ULONG_PTR)this);

	if (m_softSuspended || m_suspended)
	{
		resume();
	}

	m_finishedEvent->wait();
}

void PlatformThread::run(void(*routine)(), bool suspend, PlatformThread::Priority priority)
{
	assert(m_finished);
	if (!m_finished)
		throwf("Run called, but Thread is already running.");

	m_finished = false;
	m_suspended = suspend;

	m_handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)routine, (void*)this, 0, &m_id);
	if (!m_handle)
		throw ExceptionWinAPI();

	setPriority(priority);
}

void PlatformThread::softSuspend(bool wait)
{
	if (m_softSuspended)
		return;

	// if we're already hard-suspended, then we can't wait for an APC opportunity as it will never come. so, do nothing until resume.
	if (!m_suspended)
	{
		m_softSuspended = true;

//		ddbg << "Thread " << itostr(m_id) << " waiting for soft suspend opportunity." << ddbg.endl;
		m_softSuspendResumeEvent->reset();
		m_softSuspendReadyEvent->reset();
		QueueUserAPC(softSuspendAPCProc, m_handle, (ULONG_PTR)this);
		m_softSuspendReadyEvent->wait();
	}
	else
	{
		ddbg << "Thread " << itostr(m_id) << " ignoring soft suspend request, as it is already hard-suspended." << ddbg.endl;
	}
}

void PlatformThread::suspend()
{
	if (!m_suspended)
	{
		m_suspended = true;
		SuspendThread(m_handle);
	}
}

void PlatformThread::resume()
{
	if (m_softSuspended)
	{
		m_softSuspended = false;
		m_softSuspendResumeEvent->signal();
	}
	else if (m_suspended)
	{
		m_suspended = false;
		ResumeThread(m_handle);
		ddbg << "Thread " << itostr(m_id) << " resumed." << ddbg.endl;
	}
}

void PlatformThread::setPriority(Priority priority)
{
	if (!SetThreadPriority(m_handle, nativePriority(priority)))
		throw ExceptionWinAPI();
}
