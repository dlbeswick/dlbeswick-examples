// ---------------------------------------------------------------------------------------------------------
// 
// CMemCheck
//
// 1. Create singleton instance as early as possible
// 2. In header file common to target project, #define new MEMCHECK_NEW
// 3. Requires STLport
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once
#if USE_MEMCHECK

#include "Standard/api.h"
#include <map>
#include <vector>
#include <string>
#include "Standard/Alert.h"
#include "Standard/Help.h"
#include "Standard/Log.h"
#include "Standard/Singleton.h"

#define DELETIONS_MAX 1024
#undef min
#undef max

// CMemCheck
class CMemCheck : public Singleton<CMemCheck>
{
public:
	struct sMemEntry
	{
		sMemEntry() {};

		sMemEntry(const char* file, uint line, int amt) :
			m_file(file), m_line(line), m_amt(amt)
		{
		}

		sMemEntry(const sMemEntry& rhs)
		{
			*this = rhs;
		}

		const char* m_file;
		uint		m_line;
		uint		m_amt;

		bool operator < (const sMemEntry& rhs) const
		{
			int i = strcmp(m_file, rhs.m_file);
			return i < 0 || (i < 0 && m_line < rhs.m_line);
		}
	};

	CMemCheck() :
		m_peak(0), m_current(0)
	{
	}

	~CMemCheck()
	{
		char buf[512];

		dlog << "\n" << dlog.endl;
		dlog << "*** Memory Report ***\n" << dlog.endl;
		sprintf(buf, "Peak usage: %u bytes\n", m_peak);
		dlog << buf << dlog.endl;

		dlog << "\n" << dlog.endl;

		dlog << "*** Unfreed Memory ***\n" << dlog.endl;
		uint leak = 0;

		for (MemEntries::iterator i = m_entries.begin(); i != m_entries.end(); i++)
		{
			sMemEntry& e = i->second;

			sprintf(buf, "%s(%u): %u bytes at (%p)\n", e.m_file, e.m_line, e.m_amt, (void*)i->first);
			dlog << buf << dlog.endl;

			leak += e.m_amt;
		}

		sprintf(buf, "Total unfreed: %u bytes\n", leak);
		dlog << buf << dlog.endl;
		dlog << "\n" << dlog.endl;

		if (leak)
		{
			alert(buf, "CMemCheck");
		}
	}

	void add(void* pAddress, const sMemEntry& entry)
	{
		if (pAddress)
		{
			m_peak = std::max(m_current + entry.m_amt, m_current);
			m_current += entry.m_amt;
			m_entries[pAddress] = entry;
		}
	}

	void remove(void* pAddress)
	{
		if (pAddress)
		{
			MemEntries::iterator i = m_entries.find(pAddress);
			if (i != m_entries.end())
			{
				m_current -= i->second.m_amt;
				m_entries.erase(i);
			}
		}
	}

	void clear()
	{
		m_entries.clear();
	}

protected:
private:
	typedef std::map<void*, sMemEntry> MemEntries;
	
	MemEntries m_entries;
	uint m_current;
	uint m_peak;
};

CSINGLETON(MemCheck);


// global override new
inline void* operator new(size_t bytes, const char* file, uint line)
{
	void* pPtr = malloc(bytes);

	MemCheck().add(pPtr, CMemCheck::sMemEntry(file, line, bytes));

	return pPtr;
}

inline void* operator new(size_t bytes)
{
	void* pPtr = malloc(bytes);
	return pPtr;
}


// MSVC apparently doesn't support matching placement operator new with matching operator delete, here
// to avoid compiler warning only
inline void operator delete(void* pPtr, const char* file, uint line)
{
}


// global override delete
inline void operator delete(void* pPtr)
{
	free(pPtr);
	MemCheck().remove(pPtr);
}

// new define
#define MEMCHECK_NEW new(__FILE__, __LINE__)
#endif