#ifndef _STANDARD_MEMORYMAPPEDFILEITERATOR_H
#define _STANDARD_MEMORYMAPPEDFILEITERATOR_H

#include "Standard/api.h"
#include "Standard/MemoryMappedFileCommon.h"

class STANDARD_API MemoryMappedFile : public MemoryMappedFileCommon
{
public:
	MemoryMappedFile(const std::string& path, size_t maxSize = 0);
	~MemoryMappedFile();

	uint allocationGranularity() const;

	virtual MemoryMappedFileBufferPtr acquireView(size_t start, size_t extent);

protected:
	friend class MemoryMappedFileBufferPtr;

	virtual void releaseMappedView(const MemoryMappedFileBufferPtr& ptr);

	void doUnmap(void* buffer, size_t extent);

	int _file;
	int _allocationGranularity;
};

#endif
