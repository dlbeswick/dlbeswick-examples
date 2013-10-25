#include "Standard/pch.h"
#include "ThreadBase.h"

ThreadBase::ThreadBase(bool suspended) :
	m_finished(false),
	m_aborting(false),
	m_suspended(suspended),
	m_softSuspended(false),
	m_finishedEvent(new AsyncEvent),
	_exiting_event(new AsyncEvent)
{
}

// tbd: call 'stop' in destructor if thread is running?
ThreadBase::~ThreadBase()
{
	delete m_finishedEvent;
}

bool ThreadBase::exiting()
{
	return _exiting_event->signaled();
}

void ThreadBase::stop()
{
	stopRequest();
	waitForExit();
}

void ThreadBase::stopRequest()
{
	_exiting_event->signal();
	platformStopRequest();
}

void ThreadBase::waitForExit()
{
	platformJoin();
}
