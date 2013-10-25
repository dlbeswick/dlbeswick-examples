#ifndef STANDARD_PLATFORMTHREAD_H
#define STANDARD_PLATFORMTHREAD_H

#include "Standard/api.h"
#include "Standard/ThreadBase.h"

class STANDARD_API PlatformThread : public ThreadBase
{
public:
	PlatformThread(bool suspended);
	~PlatformThread();

	void setPriority(Priority priority);
	void suspend();
	// if "wait" is true, the calling thread will wait until the target thread has acted on the suspend request
	void softSuspend(bool wait = true);
	void resume();

protected:
	void run(void(*routine)(), bool suspend = false, Priority priority = NORMAL);

	int nativePriority(Priority p)
	{
		switch (p)
		{
		case ABOVE_NORMAL: return THREAD_PRIORITY_ABOVE_NORMAL;
		case BELOW_NORMAL: return THREAD_PRIORITY_BELOW_NORMAL;
		case HIGHEST: return THREAD_PRIORITY_HIGHEST;
		case IDLE: return THREAD_PRIORITY_IDLE;
		case LOWEST: return THREAD_PRIORITY_LOWEST;
		case TIME_CRITICAL: return THREAD_PRIORITY_TIME_CRITICAL;
		default: return THREAD_PRIORITY_NORMAL;
		}
	}

	virtual void platformStop();

	static VOID CALLBACK stopAPCProc(ULONG_PTR dwParam);
	static VOID CALLBACK softSuspendAPCProc(ULONG_PTR dwParam);

	HANDLE m_handle;
	DWORD m_id;
	bool m_finished;
	bool m_aborting;
	bool m_suspended;
	bool m_softSuspended;
	class AsyncEvent* m_softSuspendResumeEvent;
	class AsyncEvent* m_softSuspendReadyEvent;
	class AsyncEvent* m_finishedEvent;
};

#endif
