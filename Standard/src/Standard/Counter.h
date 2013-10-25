// ------------------------------------------------------------------------------------------------
//
// Counter
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Delegate.h"
#include "PendingList.h"

template <class DelegateClass>
class Counter
{
public:
	Counter() : m_count(0)
	{
	}

	typedef DelegateClass Delegate;

	void set(float time, bool once = true)
	{
		m_delegate.clear();
		m_count = time;
		m_setCount = time;
		m_once = once;
		if (m_count == 0)
			m_count = FLT_MIN;
	}
	void set(float time, const DelegateClass& delegate, bool once = true)
	{
		m_delegate = delegate;
		m_count = time;
		m_setCount = time;
		m_once = once;
		if (m_count == 0)
			m_count = FLT_MIN;
	}
	void set()
	{
		m_count = m_setCount;
		if (m_count == 0)
			m_count = FLT_MIN;
	}

	bool update(float delta)
	{
		if (m_count > 0)
		{
			m_count -= delta;
			if (m_count <= 0)
			{
				trigger();
				return true;
			}
		}

		return false;
	}

	void trigger()
	{
		m_delegate();
		if (!m_once)
			m_count = m_setCount;
		else
			m_count = 0;
	}

	bool expired() const { return m_count <= 0; }
	float operator *() const { return m_count; }
	operator bool() const { return expired(); }

protected:
	float			m_count;
	float			m_setCount;
	DelegateClass	m_delegate;
	bool			m_once;
};

// ------------------------------------------------------------------------------------------------
//
// ClassCounter
// Intended for classes that automatically track and update counters, utilise USE_COUNTERS macro.
// Instantiate counters as members of child classes.
//
// ------------------------------------------------------------------------------------------------
template <class CounterType>
class ClassCounterManager;

template <class DelegateClass, class TrackerObject>
class ClassCounter : public Counter<DelegateClass>
{
public:
	ClassCounter() :
	  m_bTracked(false),
	  m_delete(false)
	{
	}

protected:
	friend class ClassCounterManager<ClassCounter<DelegateClass, TrackerObject> >;

	void set(float time, bool once = true)
	{
		Counter<DelegateClass>::set(time, once);
	}

	void set(float time, const DelegateClass& delegate, bool once = true)
	{
		Counter<DelegateClass>::set(time, delegate, once);
	}

	void set()
	{
		Counter<DelegateClass>::set();
	}

	bool m_bTracked;
	bool m_delete;
};

// ------------------------------------------------------------------------------------------------
//
// ClassCounterManager
// Interface for clients of counter system.
//
// ------------------------------------------------------------------------------------------------

template <class CounterType>
class ClassCounterManager
{
	public:
		typedef CounterType Counter;

		void track(Counter& c)
		{
			if (!c.m_bTracked)
			{
				m_counters.add(&c);
				c.m_bTracked = true;
			}
		}

		void set(Counter& c, float time)
		{
			c.set(time, true);
			track(c);
		}

		void loop(Counter& c, float time)
		{
			c.set(time, false);
			track(c);
		}

		void set(Counter& c)
		{
			c.set();
			track(c);
		}

		void set(Counter& c, float time, const typename Counter::Delegate& d)
		{
			c.set(time, d, true);
			track(c);
		}

		void loop(Counter& c, float time, const typename Counter::Delegate& d)
		{
			c.set(time, d, false);
			track(c);
		}

		void set(float time)
		{
			CounterType* c = new CounterType;
			c->set(time, true);
			c->m_delete = true;
			track(*c);
		}

		void loop(float time)
		{
			CounterType* c = new CounterType;
			c->set(time, false);
			track(*c);
		}

		void set(float time, const typename Counter::Delegate& d)
		{
			CounterType* c = new CounterType;
			c->set(time, d, true);
			c->m_delete = true;
			track(*c);
		}

		void loop(float time, const typename Counter::Delegate& d)
		{
			CounterType* c = new CounterType;
			c->set(time, d, false);
			track(*c);
		}

		void update(float delta)
		{
			m_counters.flush();
			for (typename Counters::iterator i = m_counters.begin(); i != m_counters.end(); ++i)
			{
				CounterType* c = *i;
				if (i.added())
					break; // otherwise it's possible to form infinite loops if large delta causes timer to trigger on same frame as add, and that trigger code adds more timers

				if (c->update(delta) && c->m_delete)
				{
					m_counters.remove(c);
					delete c;
				}
			}
		}

	private:
		typedef PendingList<CounterType*> Counters;
		Counters m_counters;
};


#define USE_COUNTERS(x) \
	protected: \
		typedef ClassCounter<Delegate, x> Counter; \
		ClassCounterManager<Counter> counters; \
	private:

