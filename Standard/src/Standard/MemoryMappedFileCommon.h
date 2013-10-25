#ifndef _STANDARD_MEMORYMAPPEDFILECOMMON_H
#define _STANDARD_MEMORYMAPPEDFILECOMMON_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"
#include <assert.h>

class MemoryMappedFileCommon;

class STANDARD_API ExceptionMemoryMappedFile : public ExceptionStream
{
};

// tbd: share code with SmartPtr
class MemoryMappedFileBufferPtr
{
public:
	typedef char* P;

	// construct/copy
	MemoryMappedFileBufferPtr() :
		m_pRefCount(0),
		_extent(0),
		m_ptr(0),
		_file(0)
	{}

	MemoryMappedFileBufferPtr(MemoryMappedFileCommon& file, const P pPtr, size_t extent) :
		m_pRefCount(0),
		m_ptr(0),
		_file(&file)
	{
		assign(pPtr, extent);
	}

	MemoryMappedFileBufferPtr(const MemoryMappedFileBufferPtr& ptr) :
		_file(ptr._file)
	{
		// prevent self-assignment
		if (&ptr != this)
		{
			m_pRefCount = 0;
			m_ptr = 0;
			assign(ptr);
		}
	}

	MemoryMappedFileBufferPtr& operator = (const MemoryMappedFileBufferPtr& p)
	{
		assign(p);
		return *this;
	}

	// comparison
	bool operator == (const MemoryMappedFileBufferPtr& rhs) const	{ return m_ptr == rhs.m_ptr; }
	bool operator != (const MemoryMappedFileBufferPtr& rhs) const	{ return m_ptr != rhs.m_ptr; }
	bool operator < (const MemoryMappedFileBufferPtr& rhs) const	{ return m_ptr < rhs.m_ptr; }
	bool operator > (const MemoryMappedFileBufferPtr& rhs) const	{ return m_ptr > rhs.m_ptr; }
	bool operator == (const P& rhs) const	{ return m_ptr == rhs; }
	bool operator != (const P& rhs) const	{ return m_ptr != rhs; }
	bool operator < (const P& rhs) const	{ return m_ptr < rhs; }
	bool operator > (const P& rhs) const	{ return m_ptr > rhs; }

	// destruct
	~MemoryMappedFileBufferPtr()
	{
		if (m_pRefCount)
		{
			assert(*m_pRefCount >= 0);
		}

		decRef();
	}

	// access
	char& operator*() const { return *m_ptr; }
	P operator ->()	const { return m_ptr; }
	bool valid() const { return m_ptr != 0; }

	// query
	int* refCount() const { return m_pRefCount; }
	P const ptr() const { return m_ptr; }
	P ptr() { return m_ptr; }
	size_t extent() const { return _extent; }

protected:
	void incRef()
	{
		if (m_ptr && m_pRefCount)
			(*m_pRefCount)++;
	}

	void decRef()
	{
		if (m_ptr && m_pRefCount)
			(*m_pRefCount)--;

		validate();
	}

	void assign(const MemoryMappedFileBufferPtr& p)
	{
		decRef();

		m_pRefCount = p.refCount();
		_extent = p._extent;
		m_ptr = p.ptr();
		_file = p._file;

		incRef();
	}

	void assign(const P p, size_t extent)
	{
		decRef();

		if (p)
		{
			m_pRefCount = new int;
			*m_pRefCount = 0;

			_extent = extent;

			m_ptr = p;
			incRef();
		}
		else
		{
			m_ptr = 0;
			m_pRefCount = 0;
		}
	}

	void validate();

private:
	P		m_ptr;
	int*	m_pRefCount;
	size_t	_extent;
	class MemoryMappedFileCommon* _file;
};

class MemoryMappedFileCommon
{
public:
	MemoryMappedFileCommon(const std::string& path, size_t maxSize = 0) :
		_path(path)
	{
	}

	virtual MemoryMappedFileBufferPtr acquireView(size_t start, size_t extent) = 0;
	virtual uint allocationGranularity() const = 0;
	const std::string& path() const { return _path; }
	uint size() const { return _size; }

protected:
	friend class MemoryMappedFileBufferPtr;

	virtual void releaseMappedView(const MemoryMappedFileBufferPtr& ptr) = 0;

	void setMappedSize(size_t size) { _size = size; }
	bool shouldMapWholeFile(size_t sizeRequest) { return sizeRequest == 0; }

	uint _size;
	std::string _path;
};

void MemoryMappedFileBufferPtr::validate()
{
	if (m_pRefCount && *m_pRefCount == 0)
	{
		if (m_ptr)
		{
			_file->releaseMappedView(*this);
			m_ptr = 0;
		}

		if (m_pRefCount)
		{
			delete m_pRefCount;
			m_pRefCount = 0;
		}

		_extent = 0;
	}
}

#endif
