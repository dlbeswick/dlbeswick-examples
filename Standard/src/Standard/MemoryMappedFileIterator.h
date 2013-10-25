#ifndef _STANDARD_MemoryMappedFileIterator_H
#define _STANDARD_MemoryMappedFileIterator_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"
#include "Standard/MemoryMappedFile.h"

template <class T>
class MemoryMappedFileIterator
{
public:
	class Exception : public ExceptionStream
	{
	};

	typedef std::random_access_iterator_tag iterator_category;
	typedef T                          		value_type;
	typedef int			               		difference_type;
	typedef T*                         		pointer;
	typedef T&                         		reference;

	MemoryMappedFileIterator(MemoryMappedFile& file) :
		_buffer(0),
		_desiredStart(0xFFFFFFFF),
		_mappedStart(0xFFFFFFFF),
		_mappedSize(0xFFFFFFFF),
		_file(&file),
		_fpos(0)
	{
	}

	~MemoryMappedFileIterator()
	{
	}

	MemoryMappedFileIterator(const MemoryMappedFileIterator& rhs) :
		_mappedBuffer(rhs._mappedBuffer),
		_buffer(rhs._buffer),
		_desiredStart(rhs._desiredStart),
		_file(rhs._file),
		_fpos(rhs._fpos),
		_mappedStart(rhs._mappedStart),
		_mappedSize(rhs._mappedSize)
	{
	}

	const MemoryMappedFileIterator& operator = (const MemoryMappedFileIterator& rhs)
	{
		_buffer = rhs._buffer;
		_mappedBuffer = rhs._mappedBuffer;
		_desiredStart = rhs._desiredStart;
		_file = rhs._file;
		_fpos = rhs._fpos;
		_mappedStart = rhs._mappedStart;
		_mappedSize = rhs._mappedSize;

		return *this;
	}

	uint pageSize() const
	{
#if IS_WINDOWS
		return 1 << 20;
#else
		return _file->size();
#endif
	}

	uint bytesToMap(uint pos) const
	{
		return std::min(_file->size() - pos, pageSize());
	}

	uint fpos() const
	{
		return _fpos;
	}

	void end()
	{
		_fpos = _file->size();
	}

	T& operator*() { ensure(_fpos); return *_buffer; }
	const T& operator*() const { ensure(_fpos); return *_buffer; }
	MemoryMappedFileIterator& operator++() { inc(1); return *this; }
	MemoryMappedFileIterator& operator++(int) { MemoryMappedFileIterator result(*this); inc(1); return result; }
	MemoryMappedFileIterator& operator--() { inc(-1); return *this; }
	MemoryMappedFileIterator& operator--(int) { MemoryMappedFileIterator result(*this); inc(-1); return result; }
	void operator+=(uint amt) { inc(amt); }
	void operator-=(uint amt) { inc(-amt); }
	MemoryMappedFileIterator operator+(uint amt) const { MemoryMappedFileIterator result(*this); result += amt; return result; }
	MemoryMappedFileIterator operator-(uint amt) const { MemoryMappedFileIterator result(*this); result -= amt; return result; }
	size_t operator-(const MemoryMappedFileIterator& rhs) const { return (_fpos - rhs._fpos) / sizeof(T); }

	bool operator < (const MemoryMappedFileIterator& rhs) const { return _fpos < rhs._fpos; }
	bool operator <= (const MemoryMappedFileIterator& rhs) const { return _fpos <= rhs._fpos; }
	bool operator != (const MemoryMappedFileIterator& rhs) const { return _fpos != rhs._fpos; }
	bool operator == (const MemoryMappedFileIterator& rhs) const { return _fpos == rhs._fpos; }
	bool operator >= (const MemoryMappedFileIterator& rhs) const { return _fpos >= rhs._fpos; }
	bool operator > (const MemoryMappedFileIterator& rhs) const { return _fpos > rhs._fpos; }

private:
	uint offsetFileToBuffer(uint absolute) const
	{
		return absolute - _desiredStart;
	}

	bool isInRange(uint start, uint size) const
	{
		return start >= _mappedStart && (start + size) <= (_mappedStart + _mappedSize);
	}

	void map(uint start, uint size_bytes)
	{
		_desiredStart = start;
		uint desiredEnd = start + size_bytes;

		uint aligned_start = _file->allocationGranularity() * (start / _file->allocationGranularity());

		if (aligned_start >= _file->size())
			aligned_start = _file->size() - _file->allocationGranularity();

		size_bytes = desiredEnd - aligned_start;
		size_bytes = std::min(size_bytes, _file->size() - aligned_start);

		_mappedBuffer = _file->acquireView(aligned_start, size_bytes);
		if (!_mappedBuffer.valid())
			EXCEPTIONSTREAM(Exception(), "Couldn't acquire view of memory mapped file at offset " << aligned_start << ", extent " << size_bytes);

		_mappedStart = aligned_start;
		_mappedSize = size_bytes;

		_buffer = (T*)(_mappedBuffer.ptr() + (_desiredStart - _mappedStart));
	}

	void inc(int amt)
	{
		uint newPos = _fpos + amt * sizeof(T);
		_buffer += amt;
		_fpos = newPos;
	}

	void ensure(uint offset)
	{
		if (offset >= _file->size())
			EXCEPTIONSTREAM(Exception(), "Requested offset " << offset << " is invalid (file size " << _file->size() << ").");

		if (!isInRange(offset, sizeof(T)))
		{
			map(offset, bytesToMap(offset));
		}
	}

	// tbd: not safe for threads or multiple iterators on the same file.
	T* _buffer;
	uint _desiredStart;
	MemoryMappedFile* _file;
	uint _fpos;
	MemoryMappedFileBufferPtr _mappedBuffer;
	uint _mappedStart;
	uint _mappedSize;
};

#endif
