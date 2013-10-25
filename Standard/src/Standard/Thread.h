#ifndef STANDARD_THREAD_H
#define STANDARD_THREAD_H

#include "Standard/api.h"
#include "Standard/AsyncEvent.h"
#include "Standard/Delegate.h"
#include "Standard/PlatformThread.h"
#include "Standard/PtrGC.h"

template <class ThreadFunctorPtrFunctorClass> class ThreadFunctorPtr;

template <class DelegateClass = Delegate>
class TThread : public PlatformThread
{
public:
	TThread() :
		PlatformThread(false)
	{
	}

	TThread(DelegateClass d, bool suspend = false, Priority priority = NORMAL) :
		m_delegate(d),
		PlatformThread(suspend)
	{
		run(d, suspend, priority);
	}

	void run(DelegateClass d, bool suspend = false, Priority priority = NORMAL)
	{
		m_delegate = d;
		PlatformThread::run((void(*)())&TThread<DelegateClass>::threadFunc, suspend, priority);
	}

protected:
	static void threadFunc(void* o)
	{
		TThread<DelegateClass>* me = (TThread<DelegateClass>*)o;

		try
		{
			// we must wait until we enter threadFunc before suspending, if the suspend state was desired initially.
			// otherwise, we won't catch the ExceptionThreadAbort if the thread is stopped before it is ever resumed.
			if (me->m_suspended)
				me->suspend();

			me->m_delegate();
			me->m_finishedEvent->signal();
		}
		catch (ExceptionThreadAbort&)
		{
			me->m_finishedEvent->signal();
		}

		me->m_finished = true;
		me->self().destroy();
	}

	DelegateClass m_delegate;
};

class ThreadFunctor : public PlatformThread
{
public:
	class Functor
	{
	protected:
		friend class ThreadFunctor;

		bool exiting() { return _thread->exiting(); }

		void setOwnerThread(ThreadFunctor& owner) { _thread = &owner; }
		ThreadFunctor* _thread;
	};

protected:
	ThreadFunctor(Functor& functor) :
		PlatformThread(false)
	{
		functor.setOwnerThread(*this);
	}
};

template <class FunctorClass>
class TThreadFunctor : public ThreadFunctor
{
protected:
	template <class> friend class ThreadFunctorPtr;

	TThreadFunctor(FunctorClass functor) :
		ThreadFunctor(functor),
		_functor(functor)
	{
	}

	void run(Priority priority = NORMAL)
	{
		PlatformThread::run((void(*)())&TThreadFunctor::threadFunc, false, priority);
	}

	static void threadFunc(void* o)
	{
		TThreadFunctor* me = (TThreadFunctor*)o;

		try
		{
			me->_functor();
		}
		catch (ExceptionThreadAbort&)
		{
		}

		me->m_finished = true;
		me->self().destroy();
	}

	FunctorClass _functor;
};

template <class FunctorClass>
class ThreadFunctorPtr
{
public:
	ThreadFunctorPtr()
	{}

	bool alive() const { return _ptr.ptr() != 0; }

	void run(FunctorClass functor, ThreadBase::Priority priority = ThreadBase::NORMAL)
	{
		_ptr = PtrGC<TThreadFunctor<FunctorClass> >(new TThreadFunctor<FunctorClass>(functor));
		_ptr->run(priority);
	}

	TThreadFunctor<FunctorClass>* operator -> () { return _ptr.ptr(); }
	TThreadFunctor<FunctorClass>& operator * () { return *_ptr.ptr(); }

protected:
	PtrGC<TThreadFunctor<FunctorClass> > _ptr;
};

class Thread : public TThread<Delegate>
{
public:
	Thread(Delegate d, bool suspend = false, Priority priority = NORMAL) :
		TThread<Delegate>(d, suspend, priority)
	{
	}

protected:
	virtual ~Thread() {}
};

#endif
