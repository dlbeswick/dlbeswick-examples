#ifndef STANDARD_PLATFORMTHREAD_H
#define STANDARD_PLATFORMTHREAD_H

#include "Standard/ThreadBase.h"
#include <pthread.h>

class STANDARD_API PlatformThread : public ThreadBase
{
public:
	PlatformThread(bool suspended = false);
	~PlatformThread();
	
	virtual void setPriority(Priority priority);
	virtual void suspend();
	// if "wait" is true, the calling thread will wait until the target thread has acted on the suspend request
	virtual void softSuspend(bool wait = true);
	virtual void resume();
	
protected:
	virtual void platformStopRequest();
	virtual void platformJoin();

	void run(void(*routine)(), bool suspend, ThreadBase::Priority priority);
	
	typedef void *(*ThreadFunc)(void *);
	
	static void* thread_run_bootstrap(void* _self);

	pthread_t m_thread;

#if IS_LINUX
	ThreadFunc _client_func;
	class AsyncEvent* _thread_id_semaphore;
	pid_t _thread_id;
#endif
};

#endif
