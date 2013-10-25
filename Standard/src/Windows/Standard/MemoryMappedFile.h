#ifndef _STANDARD_MEMORYMAPPEDFILEITERATOR_H
#define _STANDARD_MEMORYMAPPEDFILEITERATOR_H

#include "Standard/api.h"
#include "Standard/MemoryMappedFileCommon.h"
#include "Standard/MemoryMappedFileIterator.h"
#include "Standard/ExceptionWinAPI.h"

class MemoryMappedFile : public MemoryMappedFileCommon
{
public:
	MemoryMappedFile(const std::string& path, size_t maxSize = 0) :
		MemoryMappedFileCommon(path, maxSize)
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		_allocationGranularity = systemInfo.dwAllocationGranularity;

		// tbd: use Path class
		if (shouldMapWholeFile(maxSize))
		{
			WIN32_FILE_ATTRIBUTE_DATA fileInfo;

			BOOL result = GetFileAttributesEx(path.c_str(), (GET_FILEEX_INFO_LEVELS)0, (void*)&fileInfo);
			if (!result)
				throw(ExceptionWinAPI(std::string("GetFileAttributesEx: ") + path));

			setMappedSize((uint)fileInfo.nFileSizeLow);
		}
		else
		{
			setMappedSize(maxSize);
		}

		_file = CreateFile(
		  path.c_str(),
		  GENERIC_WRITE | GENERIC_READ,
		  0,
		  0,
		  OPEN_ALWAYS,
		  FILE_ATTRIBUTE_NORMAL,
		  0
		);

		if (!_file)
			throw(ExceptionWinAPI(std::string("CreateFile: ") + path));

		_mapping = CreateFileMapping(_file, 0, PAGE_READWRITE, 0, maxSize, 0);
		if (!_mapping)
			throw(ExceptionWinAPI(std::string("CreateFileMapping: ") + path));
	}

	~MemoryMappedFile()
	{
		CloseHandle(_mapping);
		CloseHandle(_file);
	}

	uint allocationGranularity() const { return _allocationGranularity; }
	HANDLE mapping() const { return _mapping; }

	virtual MemoryMappedFileBufferPtr acquireView(size_t start, size_t extent)
	{
		void* result = (char*)MapViewOfFile(mapping(), FILE_MAP_WRITE | FILE_MAP_READ, 0, start, extent);
		if (!result)
			throw(ExceptionWinAPI(std::string("MapViewOfFile: ") + path()));

		return MemoryMappedFileBufferPtr(*this, (char*)result);
	}

protected:
	friend class MemoryMappedFileBufferPtr;

	virtual void releaseMappedView(void* buffer)
	{
		UnmapViewOfFile(buffer);
	}

	uint _allocationGranularity;
	HANDLE _mapping;
	HANDLE _file;
};

#endif
