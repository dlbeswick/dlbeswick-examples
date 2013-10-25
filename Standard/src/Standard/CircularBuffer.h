#pragma once

#include "Standard/api.h"

class CircularBufferBase
{
public:
	static int distance(int start, int end, int bufferSize)
	{
		if (end >= start)
			return end - start;
		else
			return (bufferSize - start) + end;
	}

	static bool passed(int oldp, int mark, int newp)
	{
		return (oldp <= mark && mark <= newp)
			|| (mark <= newp && newp < oldp)
			|| (newp < oldp && oldp <= mark);
	}
};

template <class T, int ElementSize = 1>
class CircularBufferPtr : public CircularBufferBase
{
public:
	CircularBufferPtr(T* ptr = 0, int size = 0)
	{
		set(ptr, size);
	}

	void advance(int& cursor, int elements)
	{
		cursor = (cursor + elements) % m_size;
	}

	void set(T* ptr, int size)
	{
		m_ptr = ptr;
		m_size = size;
	}

	void copyFrom(int destPos, const T* buf, int size)
	{
		T* source[2];
		int sourceSize[2];
		get(destPos, size, source[0], sourceSize[0], source[1], sourceSize[1]);

		memcpy(source[0], buf, sourceSize[0] * sizeof(T) * ElementSize);
		if (sourceSize[1])
		{
			memcpy(source[1], buf + sourceSize[0], sourceSize[1] * sizeof(T) * ElementSize);
		}
	}

	void copyTo(int sourcePos, T* buf, int size)
	{
		T* source[2];
		int sourceSize[2];
		get(sourcePos, size, source[0], sourceSize[0], source[1], sourceSize[1]);

		memcpy(buf, source[0], sourceSize[0] * sizeof(T) * ElementSize);
		if (sourceSize[1])
		{
			memcpy(buf + sourceSize[0] * ElementSize, source[1],  sourceSize[1] * sizeof(T) * ElementSize);
		}
	}

	void copy(int sourcePos, CircularBufferPtr<T>& dest, int destPos, int size)
	{
		T* sourceBuf[2];
		int sourceSize[2];
		get(sourcePos, size, sourceBuf[0], sourceSize[0], sourceBuf[1], sourceSize[1]);

		T* destBuf[2];
		int destSize[2];
		dest.get(destPos, size, destBuf[0], destSize[0], destBuf[1], destSize[1]);

		int sourceIdx = 0;
		int destIdx = 0;
		while (sourceIdx < 2 && destIdx < 2)
		{
			int copy = std::min(sourceSize[sourceIdx], destSize[destIdx]);
			memcpy(destBuf[destIdx], sourceBuf[sourceIdx], copy * sizeof(T) * ElementSize);
			sourceSize[sourceIdx] -= copy;
			destSize[destIdx] -= copy;
			destBuf[destIdx] += copy;

			while (sourceSize[sourceIdx] == 0)
				++sourceIdx;

			while (destSize[destIdx] == 0)
				++destIdx;
		}
	}

	T* get(int start)
	{
		return m_ptr + start * ElementSize;
	}

	// returns the entirety of the requested size in buf0/size0 if the requested range does not extend beyond the
	// range of the buffer, otherwise returns the range up to the end of the buffer in buf0/size0 and the remainder in
	// buf1/size1. the maximum allowable request size is the total length of the buffer.
	void get(int start, int num, T*& buf0, int& size0, T*& buf1, int& size1)
	{
		if (num > (int)m_size)
		{
			assert(0);
			num = m_size;
		}

		T* bufferStart = m_ptr;
		T* bufferEnd = bufferStart + m_size * ElementSize;

		buf0 = bufferStart + start * ElementSize;
		assert(buf0 <= bufferEnd);

		if (num * ElementSize <= bufferEnd - buf0)
		{
			size0 = num;

			assert(buf0 + size0 * ElementSize <= bufferEnd);

			buf1 = 0;
			size1 = 0;
		}
		else
		{
			size0 = (bufferEnd - buf0) / ElementSize;

			buf1 = bufferStart;
			size1 = (num - size0);
		}
	}

	void get(int start, int num, T const*& buf0, int& size0, T const*& buf1, int& size1) const
	{
		return ((CircularBufferPtr*)this)->get(start, num, (T*&)buf0, size0, (T*&)buf1, size1);
	}

	int size() const
	{
		return m_size;
	}

	T* buf()
	{
		return m_ptr;
	}

	const T* buf() const
	{
		return m_ptr;
	}

	int distance(int start, int end) const
	{
		assert(start <= m_size);
		assert(end <= m_size);

		if (end >= start)
			return end - start;
		else
			return (m_size - start) + end;
	}

	// returns the amount of overlap in elements if the range defined by start1->end1 intersects with the range start0->end0
	int overlap(int start0, int end0, int start1, int end1)
	{
		int size0 = distance(start0, end0);
		int size1 = distance(start1, end1);

		if (distance(start0, start1) > size0)
			return 0;

		return std::min(distance(start1, end0), size1);
	}

protected:
	int m_size;
	T* m_ptr;
};

template <class T, int ElementSize = 1>
class CircularBuffer : public CircularBufferPtr<T, ElementSize>
{
public:
	CircularBuffer(int startSize = 0)
	{
		resize(startSize);
	}

	void add(const T& item)
	{
		m_buffer.push_back(item);
	}

	void resize(int size)
	{
		m_buffer.resize(size * ElementSize);
		this->m_ptr = &*m_buffer.begin();
		this->m_size = size;
	}

protected:
	std::vector<T> m_buffer;
};
