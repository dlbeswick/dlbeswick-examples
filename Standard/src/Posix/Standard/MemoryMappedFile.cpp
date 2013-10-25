#include "Standard/MemoryMappedFile.h"
#include "Standard/ExceptionPosix.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <memory.h>

MemoryMappedFile::MemoryMappedFile(const std::string& input_path, size_t maxSize) :
	MemoryMappedFileCommon(input_path, maxSize),
	_file(-1)
{
	struct stat file_stat;

	int result = stat(path().c_str(), &file_stat);

	bool fileExist = result == 0;

	// if stat failed for any reason besides a missing file...
	if (result != 0 && errno != ENOENT)
		EXCEPTIONSTREAM(ExceptionPosix(), "MemoryMappedFile: stat failed for " << path());

	_allocationGranularity = getpagesize();

	// tbd: use Path class
	if (shouldMapWholeFile(maxSize))
	{
		if (!fileExist)
			EXCEPTIONSTREAM(ExceptionMemoryMappedFile(), "MemoryMappedFile: max size must be specified if file doesn't exist. File path: " << path());

		setMappedSize((uint)file_stat.st_size);
	}
	else
	{
		setMappedSize(maxSize);
	}

	_file = open(path().c_str(), O_RDWR | O_CREAT, 0644);
	if (_file == -1)
		EXCEPTIONSTREAM(ExceptionPosix(), "MemoryMappedFile: fopen failed for " << path());

	// prepare an empty file by filling it with zeroes
	if (!fileExist)
	{
		assert(maxSize > 0);

		char zero[4096];
		memset(zero, 0, sizeof(zero));

		size_t remaining = maxSize;
		while (remaining != 0)
		{
			size_t toWrite = std::min(remaining, sizeof(zero));
			write(_file, &zero, toWrite);
			remaining -= toWrite;
		}
	}
}

MemoryMappedFile::~MemoryMappedFile()
{
	if (_file != -1)
	{
		int result = close(_file);
		if (result == -1)
			EXCEPTIONSTREAM(ExceptionPosix(), "MemoryMappedFile: close failed for " << path());
	}
}

void MemoryMappedFile::doUnmap(void* buffer, size_t extent)
{
	if (munmap(buffer, extent) == -1)
		EXCEPTIONSTREAM(ExceptionPosix(), "MemoryMappedFile: munmap failed for " << path());
}

uint MemoryMappedFile::allocationGranularity() const
{
	return _allocationGranularity;
}

MemoryMappedFileBufferPtr MemoryMappedFile::acquireView(size_t start, size_t extent)
{
	void* result = mmap(0, extent, PROT_READ | PROT_WRITE, MAP_SHARED, _file, start);
	if (result == MAP_FAILED)
		EXCEPTIONSTREAM(ExceptionPosix(), "MemoryMappedFile: mmap with start '" << start << "', extent '" << extent << "' failed for " << path());

	return MemoryMappedFileBufferPtr(*this, (char*)result, extent);
}

void MemoryMappedFile::releaseMappedView(const MemoryMappedFileBufferPtr& ptr)
{
	doUnmap(ptr.ptr(), ptr.extent());
}

