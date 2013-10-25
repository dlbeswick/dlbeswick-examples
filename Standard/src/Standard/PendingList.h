// ---------------------------------------------------------------------------------------------------------
//
// PendingList
// TODO: structure of list never changes between flushes -- removes can be implemented by storing iterators
//
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Help.h"
#include <vector>

#if IS_MSVC
#pragma inline_depth(255)
#endif

#if IS_GNUC
#include <stddef.h>
#endif

namespace Pending
{
	template <class T>
	struct Container
	{
		Container()
		{
		}

		Container(const T& _item)
		{
			bRemove = false;
			item = _item;
		}

		bool operator == (const Container& rhs) const { return item == rhs.item; }

		T item;
		bool bRemove;
	};

	// standard remove
	template <class T>
	class RemoveFunc : public std::binary_function<typename std::vector<Container<T> >, typename std::vector<Container<T> >::iterator, void>
	{
	public:
		void operator()(std::vector<Container<T> >& list, const typename std::vector<Container<T> >::iterator& i)
		{
			list.erase(i);
		}
	};

	// "owner pointer" items -- remove and delete
	template <class T>
	class OwnerRemove : public RemoveFunc<T>
	{
	public:
		void operator()(std::vector<Container<T> >& list, typename std::vector<Container<T> >::iterator& i)
		{
			delete *i;
			list.erase(i);
		}
	};

	// iterators /////////////////////////////////////////////////
	class Fwd
	{
	public:
		static void inc(int& idx)
		{
			++idx;
		}

		static void dec(int& idx)
		{
			--idx;
		}
	};

	class Rev
	{
	public:
		static void inc(int& idx)
		{
			--idx;
		}

		static void dec(int& idx)
		{
			++idx;
		}
	};

	template <class T, class ItType, class Policy>
	class PendingListIteratorTemplate : public std::iterator<std::random_access_iterator_tag, Container<T>, ptrdiff_t, T*, T& >
	{
	public:
		typedef std::vector<Container<T> > List;

		operator ItType ()
		{
			return *(ItType*)this;
		}

		void remove() { ((ItType*)this)->itemIdx().bRemove = true; }
		bool removed() const { return ((ItType*)this)->validIdx() && ((ItType*)this)->itemIdx().bRemove; }
		bool added() const { return ((ItType*)this)->validIdx() && idx >= (int)items.size(); }
		typename List::iterator internal() const { return ((ItType*)this)->itemIdx(); }

		int internalIdx() const { return idx; }
		void internalSetIdx(int i) { idx = i; }

		__forceinline ItType& operator++ () { ((ItType*)this)->inc(); return (ItType&)*this; }
		__forceinline ItType operator++ (int) { ItType i((ItType&)*this); ((ItType*)this)->inc(); return i; }
		__forceinline ItType& operator-- () { ((ItType*)this)->ItType::dec(); return (ItType&)*this; }
		__forceinline ItType operator-- (int) { ItType i((ItType&)*this); ((ItType*)this)->dec(); return i; }
		__forceinline ItType operator + (int i) const { ItType it((ItType&)*this); for (; i > 0; --i) it.inc(); return it; }
		__forceinline ItType operator - (int i) const { ItType it((ItType&)*this); for (; i > 0; --i) it.dec(); return it; }

		bool operator == (const ItType& rhs) const { return idx == rhs.idx; }
		bool operator != (const ItType& rhs) const { return idx != rhs.idx; }
		bool operator < (const ItType& rhs) const { return idx < rhs.idx; }
		bool operator > (const ItType& rhs) const { return idx > rhs.idx; }
		bool operator - (const ItType& rhs) const { return idx - rhs.idx; }

	protected:
		PendingListIteratorTemplate(List& _items, List& _add) :
			items(_items), add(_add), idx(-1)
		{
		}

		PendingListIteratorTemplate(List& _items, List& _add, int _idx) :
			items(_items), add(_add), idx(_idx)
		{
		}

		__forceinline void inc()
		{
			do
			{
				Policy::inc(idx);
			}
			while (((ItType*)this)->validIdx() && ((ItType*)this)->itemIdx().bRemove);
		}

		__forceinline void dec()
		{
			do
			{
				Policy::dec(idx);
			}
			while (((ItType*)this)->validIdx() && ((ItType*)this)->itemIdx().bRemove);
		}

		Container<T>& itemIdx()
		{
			if (idx < (int)items.size())
				return items[idx];
			else
				return add[idx - (int)items.size()];
		}

		bool validIdx() const
		{
			return idx < (int)items.size() + (int)add.size() && idx >= 0;
		}

		int idx;

#if IS_GNUC
		List& items;
		List& add;
#else
		// tbd: check if this is necessary
		mutable List& items;
		mutable List& add;
#endif
	};

	template <class T, class Policy>
	class PendingListConstIterator : public PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>
	{
	public:
		typedef typename PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>::List List;

		PendingListConstIterator(const List& _items, const List& _add, int _idx) :
			PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>((List&)_items, (List&)_add, _idx)
		{
		}

		PendingListConstIterator(const List& _items, const List& _add) :
			PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>((List&)_items, (List&)_add)
		{
			this->inc();
		}

		const T& operator * () { return this->itemIdx().item; }
		const T* operator -> () { return &this->itemIdx().item; }
	};

	template <class T, class Policy>
	class PendingListIterator : public PendingListIteratorTemplate<T, PendingListIterator<T, Policy>, Policy>
	{
	public:
		typedef typename PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>::List List;

		PendingListIterator(List& _items, List& _add, int _idx) :
			PendingListIteratorTemplate<T, PendingListIterator<T, Policy>, Policy>(_items, _add, _idx)
		{
		}

		PendingListIterator(List& _items, List& _add) :
			PendingListIteratorTemplate<T, PendingListIterator<T, Policy>, Policy>(_items, _add)
		{
			this->inc();
		}

		T& operator * () { return this->itemIdx().item; }
		T* operator -> () { return &this->itemIdx().item; }
	};

	// null remover iterators
	template <class T, class Policy>
	class NullRemoverPendingListIterator : public PendingListIteratorTemplate<T, NullRemoverPendingListIterator<T, Policy>, Policy>
	{
	public:
		typedef typename PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>::List List;

		NullRemoverPendingListIterator(List& _items, List& _add, int _idx) :
			PendingListIteratorTemplate<T, NullRemoverPendingListIterator<T, Policy>, Policy>(_items, _add, _idx)
		{
		}

		NullRemoverPendingListIterator(List& _items, List& _add) :
			PendingListIteratorTemplate<T, NullRemoverPendingListIterator<T, Policy>, Policy>(_items, _add)
		{
			this->inc();
		}

		T& operator * () { return this->itemIdx().item; }
		T* operator -> () { return &this->itemIdx().item; }

		__forceinline void inc()
		{
			do
			{
				Policy::inc(this->idx);
			}
			while (this->validIdx() && (this->itemIdx().bRemove || !this->itemIdx().item));
		}

		__forceinline void dec()
		{
			do
			{
				Policy::dec(this->idx);
			}
			while (this->validIdx() && (this->itemIdx().bRemove || !this->itemIdx().item));
		}
	};

	template <class T, class Policy>
	class NullRemoverPendingListConstIterator : public PendingListIteratorTemplate<T, NullRemoverPendingListConstIterator<T, Policy>, Policy>
	{
	public:
		typedef typename PendingListIteratorTemplate<T, PendingListConstIterator<T, Policy>, Policy>::List List;

		NullRemoverPendingListConstIterator(const List& _items, const List& _add, int _idx) :
			PendingListIteratorTemplate<T, NullRemoverPendingListConstIterator<T, Policy>, Policy>((List&)_items, (List&)_add, _idx)
		{
		}

		NullRemoverPendingListConstIterator(const List& _items, const List& _add) :
			PendingListIteratorTemplate<T, NullRemoverPendingListConstIterator<T, Policy>, Policy>((List&)_items, (List&)_add)
		{
			this->inc();
		}

		const T& operator * () { return this->itemIdx().item; }
		const T* operator -> () { return &this->itemIdx().item; }

		__forceinline void inc()
		{
			do
			{
				Policy::inc(this->idx);
			}
			while (this->validIdx() && (this->itemIdx().bRemove || !this->itemIdx().item));
		}

		__forceinline void dec()
		{
			do
			{
				Policy::dec(this->idx);
			}
			while (this->validIdx() && (this->itemIdx().bRemove || !this->itemIdx().item));
		}
	};

	// sorting
	template <class T>
	class Ordering
	{
	public:
		typedef std::vector<Pending::Container<T> > Items;

		typename Items::iterator insert(Items& items, const Container<T>& item) const
		{
			return items.insert(items.end(), item);
		}

		typename Items::iterator find(const Items& items, const Container<T>& item) const
		{
			return std::find(items.begin(), items.end(), item);
		}
	};

	template <class T>
	class Unsorted : public Ordering<T>
	{
	public:
		typename Ordering<T>::Items::iterator insert(typename Ordering<T>::Items& items, const Container<T>& item) const
		{
			return items.insert(items.end(), item);
		}

		typename Ordering<T>::Items::iterator find(typename Ordering<T>::Items& items, const Container<T>& item) const
		{
			return std::find(items.begin(), items.end(), item);
		}
	};

	template <class T, class Pred>
	class BinarySort : public Ordering<T>
	{
	public:
		class PredicateAdapter
		{
		public:
			bool operator () (const Container<T>& lhs, const Container<T>& rhs) const
			{
				return Pred()(lhs.item, rhs.item);
			}
		};

		typename Ordering<T>::Items::iterator insert(typename Ordering<T>::Items& items, const Container<T>& item) const
		{
			return items.insert(std::lower_bound(items.begin(), items.end(), item, PredicateAdapter()), item);
		}

		typename Ordering<T>::Items::iterator find(typename Ordering<T>::Items& items, const Container<T>& item) const
		{
			return std::lower_bound(items.begin(), items.end(), item, PredicateAdapter());
		}
	};
};

//////////////////////////////////////////////////////////////////

template <class T, class RemoveFunc = Pending::RemoveFunc<T>, class SortFunc = Pending::Unsorted<T> >
class PendingList
{
public:
	typedef std::vector<Pending::Container<T> > Items;
	typedef Pending::PendingListIterator<T, Pending::Fwd> iterator;
	typedef Pending::PendingListConstIterator<T, Pending::Fwd> const_iterator;
	typedef Pending::PendingListIterator<T, Pending::Rev> reverse_iterator;

	iterator begin() { return iterator(m_items, m_addList); }
	iterator end() { return iterator(m_items, m_addList, m_items.size() + m_addList.size()); }
	const_iterator begin() const { return const_iterator(m_items, m_addList); }
	const_iterator end() const { return const_iterator(m_items, m_addList, m_items.size() + m_addList.size()); }

	reverse_iterator rbegin()
	{
		reverse_iterator r(m_items, m_addList, m_items.size() + m_addList.size());
		++r;
		return r;
	}
	reverse_iterator rend()
	{
		return reverse_iterator(m_items, m_addList, -1);
	}

	~PendingList()
	{
		clear();
	}

    void add(const T& item)
	{
		m_addList.push_back(Pending::Container<T>(item));
	}

	// use only for efficiency reasons inside "item" iterator loops
	void erase(const iterator& i)
	{
		m_items.erase(i);
	}

	int remove(const T& item)
	{
		int removed = false;

		typename Items::iterator i;

		i = SortFunc().find(m_addList, Pending::Container<T>(item));
		if (i != m_addList.end())
		{
			i->bRemove = true;
			++removed;
		}

		i = SortFunc().find(m_items, Pending::Container<T>(item));
		if (i != m_items.end())
		{
			i->bRemove = true;
			++removed;
		}

		return removed;
	}

	inline void flush();

	uint size() const { return m_items.size() + m_addList.size(); }
	bool empty() const { return m_items.empty() && m_addList.empty(); }

	void clear()
	{
		flush();
		while (!m_items.empty())
		{
			m_remove(m_items, m_items.end() - 1);
		}
	}

	Items& items() { return m_items; }
	const Items& items() const { return m_items; }
	Items& addList() { return m_addList; }
	const Items& addList() const { return m_addList; }

	__forceinline T& operator[](int idx) { return *(begin() + idx); }
	__forceinline const T& operator[](int idx) const { return *(begin() + idx); }

	bool contains(const T& item) const
	{
		typename Items::const_iterator i;

		i = std::find(m_addList.begin(), m_addList.end(), Pending::Container<T>(item));
		if (i != m_addList.end() && !i->bRemove)
		{
			return true;
		}

		i = std::find(m_items.begin(), m_items.end(), Pending::Container<T>(item));
		if (i != m_items.end() && !i->bRemove)
		{
			return true;
		}

		return false;
	}

protected:
	inline void removeElements();
	inline void addElements();

	Items m_addList;
	Items m_items;
	RemoveFunc m_remove;
};

template <class T>
class OwnerPendingList : public PendingList<T, Pending::OwnerRemove<T> >
{
};

template <class T, class SortFunc = Pending::Unsorted<T> >
class PendingListNullRemover : public PendingList<T, Pending::RemoveFunc<T>, SortFunc>
{
public:
	typedef Pending::NullRemoverPendingListIterator<T, Pending::Fwd> iterator;
	typedef Pending::NullRemoverPendingListConstIterator<T, Pending::Fwd> const_iterator;
	typedef Pending::NullRemoverPendingListIterator<T, Pending::Rev> reverse_iterator;
	typedef typename PendingList<T, Pending::RemoveFunc<T>, SortFunc>::Items Items;

	iterator begin() { return iterator(this->m_items, this->m_addList); }
	iterator end() { return iterator(this->m_items, this->m_addList, this->m_items.size() + this->m_addList.size()); }
	const_iterator begin() const { return const_iterator(this->m_items, this->m_addList); }
	const_iterator end() const { return const_iterator(this->m_items, this->m_addList, this->m_items.size() + this->m_addList.size()); }

	reverse_iterator rbegin()
	{
		reverse_iterator r(this->m_items, this->m_addList, this->m_items.size() + this->m_addList.size());
		++r;
		return r;
	}
	reverse_iterator rend()
	{
		return reverse_iterator(this->m_items, this->m_addList, -1);
	}

	inline void flush()
	{
		this->addElements();

		for (typename Items::iterator i = this->m_items.begin(); i != this->m_items.end(); )
		{
			if (!i->item || i->bRemove)
			{
				this->m_remove(this->m_items, i);
			}
			else
				++i;
		}
	}
};

// implementation
template <class T, class RemoveFunc, class SortFunc>
void PendingList<T, RemoveFunc, SortFunc>::removeElements()
{
	// remove pending elements
	// TBD: no items are removed from m_items between flushes, so a list of iterators pointing to elements for removal can be used instead
	for (typename Items::iterator i = this->m_items.begin(); i != this->m_items.end(); )
	{
		if (i->bRemove)
		{
			this->m_items.erase(i);
		}
		else
			++i;
	}
}

template <class T, class RemoveFunc, class SortFunc>
void PendingList<T, RemoveFunc, SortFunc>::addElements()
{
	Items& add = m_addList;
	Items& list = m_items;

	// add pending elements
	if (!add.empty())
	{
		for (typename Items::iterator i = add.begin(); i != add.end(); ++i)
		{
			if (!i->bRemove)
			{
				SortFunc().insert(list, *i);
			}
		}

		add.resize(0);
	}
}

template <class T, class RemoveFunc, class SortFunc>
void PendingList<T, RemoveFunc, SortFunc>::flush()
{
	removeElements();
	addElements();
}

