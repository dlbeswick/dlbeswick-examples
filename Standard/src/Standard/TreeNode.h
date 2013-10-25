// ------------------------------------------------------------------------------------------------
//
// TreeNode
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "PendingList.h"

template <class T, class ListType = PendingListNullRemover<T> >
class TreeNode
{
public:
	typedef ListType Children;

	virtual const PtrGC<Object>& parent() const { return m_parent; }

	virtual void setParent(const PtrGC<Object>& p)
	{
		if (m_parent)
		{
			m_parent->removeChildReference(this);
		}

		m_parent = p;

		if (p)
		{
			m_parent->addChildReference(this);
		}
	}

	virtual const Children& children() const { return m_children; }

	void flushChildren()
	{
		m_children.flush();
	}

protected:
	virtual void addChild(const T& p) { m_children.add(p); }
	virtual void removeChild(const T& p) { m_children.remove(p); }

	Children	m_children;
	T			m_parent;
};


template <class T, class ListType = PendingListNullRemover<T> >
class MultiTreeNode
{
public:
	typedef ListType Children;
	typedef ListType Parents;

	virtual void addChild(const T& p) { m_children.add(p); }
	virtual void removeChild(const T& p) { m_children.remove(p); }

	const Parents& parents() const { return m_parents; }
	const Children& children() const { return m_children; }

	void flushChildren()
	{
		m_children.flush();
		m_parents.flush();
	}

protected:
	Children	m_children;
	Parents		m_parent;
};