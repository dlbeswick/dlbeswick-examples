#pragma once
#include "BufferModifier.h"
#include <Standard/SortedVec.h>

class BufferModifierHost
{
public:
	virtual ~BufferModifierHost() {}

	virtual void addModifier(BufferModifier& modifier, int priority = 0)
	{
        Critical(_critical);
		modifier.m_priority = priority;
		modifier.setHost(this);
		m_modifiers.insert(&modifier);
	}

	virtual void removeModifier(BufferModifier& modifier)
	{
        Critical(_critical);
		m_modifiers.erase(std::lower_bound(m_modifiers.begin(), m_modifiers.end(), &modifier));
	}

	template <class T>
	bool applyModifiers(typename T::SampleElement*& buf, int& samples, int stride)
	{
		bool wasModified = false;

        Modifiers mods;
        
        {
            Critical(_critical);
            mods = m_modifiers;
        }
        
		for (Modifiers::iterator i = m_modifiers.begin(); i != m_modifiers.end(); ++i)
		{
			BufferModifier& m = **i;

			if (m.enabled())
			{
				bool modifiedBuffer = m.modify(buf, samples, stride);

				wasModified = wasModified || modifiedBuffer;
			}
		}

		return wasModified;
	}

	void duplicateModifiers(BufferModifierHost& dest) const
	{
        Critical(_critical);
		dest.m_modifiers = m_modifiers;
	}
    
    bool hasModifiers() const
    {
        Critical(_critical);
        return !m_modifiers.empty();
    }
    
    typedef std::vector<BufferModifier*> SortedModifiers;
    SortedModifiers modifiers() const
    {
        SortedModifiers result;

        {
            Critical(_critical);
            result.insert(result.begin(), m_modifiers.begin(), m_modifiers.end());
        }
        
        return result;
    }

protected:
	BufferModifierHost()
	{
	}

	typedef SortedVec<BufferModifier*, DereferenceLessThan<BufferModifier*> > Modifiers;
	Modifiers m_modifiers;
	std::vector<char> m_circularWorkArea;
    CriticalSection _critical;
};
