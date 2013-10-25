#include "Standard/pch.h"
#include "Standard/AsyncEvent.h"
#include "Standard/Log.h"
#include "PlatformThread.h"

#if IS_LINUX
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#endif

PlatformThread::PlatformThread(bool suspended) :
	ThreadBase(suspended),
	m_thread(0),
	_thread_id_semaphore(new AsyncEvent)
{
}

PlatformThread::~PlatformThread()
{
	if (m_thread)
		stop();

	delete _thread_id_semaphore;
}

void PlatformThread::setPriority(ThreadBase::Priority priority)
{
#if IS_LINUX
	// Note: this will only work properly in linux.
	int nice_value = 0;

	switch (priority)
	{
	case ABOVE_NORMAL:
		nice_value = -5;
	case BELOW_NORMAL:
		nice_value = 5;
	case HIGHEST:
		nice_value = -10;
	case IDLE:
		nice_value = 19;
	case LOWEST:
		nice_value = 18;
	case NORMAL:
		nice_value = 0;
	case TIME_CRITICAL:
		nice_value = -11;
	};

	_thread_id_semaphore->wait();
	int ret = setpriority(PRIO_PROCESS, _thread_id, nice_value);
	if (ret != 0)
		derr << "PlatformThread::setPriority: setPriority call failed." << derr.endl;
#else
	throwf("PlatformThread::suspend(): not yet supported on this platform.");
#endif
}

// suspend should probably be completely removed. synchronisation structures should be used to pause and resume execution.
void PlatformThread::suspend()
{
	throwf("PlatformThread::suspend(): not yet supported.");
}

// suspend should probably be completely removed. synchronisation structures should be used to pause and resume execution.
void PlatformThread::softSuspend(bool wait)
{
	throwf("PlatformThread::suspend(): not yet supported.");
}

// suspend should probably be completely removed. synchronisation structures should be used to pause and resume execution.
void PlatformThread::resume()
{
	throwf("PlatformThread::suspend(): not yet supported.");
}

void PlatformThread::platformStopRequest()
{
	_thread_id_semaphore->wait();

	dlog << "Stopping thread " << _thread_id << ", waiting for it to finish..." << dlog.endl;
}

void PlatformThread::platformJoin()
{
	pthread_join(m_thread, 0);
	dlog << "Thread " << _thread_id << "stopped." << dlog.endl;

	m_thread = 0;
}

void PlatformThread::run(void(*routine)(), bool suspend, ThreadBase::Priority priority)
{
	_client_func = (ThreadFunc)routine;

	pthread_create(&m_thread, 0, &PlatformThread::thread_run_bootstrap, (void*)this);

	if (priority != NORMAL)
		setPriority(priority);
}

void* PlatformThread::thread_run_bootstrap(void* _self)
{
	PlatformThread* self = (PlatformThread*)_self;

#if IS_LINUX
	self->_thread_id = syscall(SYS_gettid);
	self->_thread_id_semaphore->signal();
#endif

	return (*self->_client_func)(self);
}
