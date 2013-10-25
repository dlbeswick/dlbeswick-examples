// AsyncWait
// Thread-safe signalling classes used to trigger events among processes.
//
// Example usage:
//
//AsyncEvent event;
//
//void thread1()
//{
//	while (true)
//	{
//		printf("I'm	thread1, and I'm waiting for thread2.");
//		{
//			AsyncWait(event);
//			printf("I'm	thread1, and thread2 just signalled.");
//		}
//	}
//}
//
//void thread2()
//{
//	while (true)
//	{
//		Sleep(1000);
//		event.signal();
//	}
//}

#ifndef STANDARD_ASYNCEVENT_H
#define STANDARD_ASYNCEVENT_H

#include "Standard/api.h"
#include "Standard/CriticalSection.h"
#include "Standard/Exception.h"

class STANDARD_API AsyncWaitable
{
public:
	// returns false if wait was aborted for any reason besides event signal (i.e. APC event)
	virtual bool wait(int ms = INFINITE) = 0;
	virtual void onWaitEnded() {}
	virtual void onExclusiveEnter() {}
	virtual void onExclusiveExit() {}

protected:
	AsyncWaitable() {}
};

class STANDARD_API AsyncWaitBlock
{
public:
	AsyncWaitBlock(AsyncWaitable& event);
	~AsyncWaitBlock();

protected:
	AsyncWaitable& m_event;
};

#define AsyncWait(x) AsyncWaitBlock asyncwait(x)

// "Critical Section" emulation using "WaitForSingleObjectEx", to allow interrupt during critical section wait.
// opposite of "AsyncWait" -- signals event during block and unsignals at end
class STANDARD_API AsyncExclusiveBlock
{
public:
	AsyncExclusiveBlock(AsyncWaitable& event);
	~AsyncExclusiveBlock();

protected:
	AsyncWaitable& m_event;
};

#define AsyncExclusive(x) AsyncExclusiveBlock asyncexclusive(x)

////

class STANDARD_API AsyncEvent : public AsyncWaitable
{
public:
	AsyncEvent(bool signaled = false);
	virtual ~AsyncEvent();

	void signal();
	bool signaled();
	void reset();
	virtual bool wait(int ms = INFINITE);
	virtual void onWaitEnded();
	virtual void onExclusiveEnter();
	virtual void onExclusiveExit();
	HANDLE handle();

protected:
	CriticalSection m_signaledMutex;
	bool m_signaled;
	HANDLE m_handle;
};

class STANDARD_API AsyncEvents : public AsyncWaitable
{
public:
	AsyncEvents(int count, bool signaled = false);
	virtual ~AsyncEvents();

	void signal(int i);
	void signalAll();
	void reset(int i);
	int activatedIdx() const { return m_activated; }
	const HANDLE& handle(int i);
	int count() const { return m_count; }

	virtual bool wait(int ms = INFINITE);

	bool wait(bool all = false, int ms = INFINITE);
	virtual void onWaitEnded();

protected:
	bool m_signaled;
	int m_activated;
	int m_count;
	HANDLE* m_handles;
};

#endif
