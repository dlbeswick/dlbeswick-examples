#ifndef STANDARD_THREADBASE_H
#define STANDARD_THREADBASE_H

#include "Standard/api.h"
#include "Standard/AsyncEvent.h"
#include "Standard/Delegate.h"
#include "Standard/Exception.h"
#include "Standard/PtrGCHost.h"

class STANDARD_API ExceptionThreadAbort : public Exception
{
public:
	virtual const char* what() const throw() { return "Thread stop requested."; }
};

class STANDARD_API ThreadBase : public PtrGCHost
{
public:
	ThreadBase(bool suspended);
	virtual ~ThreadBase();
	
	enum Priority
	{
		ABOVE_NORMAL,
		BELOW_NORMAL,
		HIGHEST,
		IDLE,
		LOWEST,
		NORMAL,
		TIME_CRITICAL
	};
	
	virtual bool exiting();
	virtual void setPriority(Priority priority) = 0;
	virtual void suspend() = 0;
	// if "wait" is true, the calling thread will wait until the target thread has acted on the suspend request
	virtual void softSuspend(bool wait = true) = 0;
	virtual void resume() = 0;
	// Asks the thread to exit and waits for completion.
	virtual void stop();
	// Asks the thread to exit and allows execution to proceed.
	virtual void stopRequest();
	// Waits for the thread to exit after a call to stopRequest.
	virtual void waitForExit();
		
protected:
	virtual void run(void(*routine)(), bool suspend = false, Priority priority = NORMAL) = 0;
	
	virtual void platformStopRequest() = 0;
	virtual void platformJoin() = 0;

	bool m_finished;
	bool m_aborting;
	bool m_suspended;
	bool m_softSuspended;
	class AsyncEvent* m_finishedEvent;
	class AsyncEvent* _exiting_event;
};

#endif
