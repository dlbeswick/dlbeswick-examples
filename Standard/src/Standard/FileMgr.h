// ---------------------------------------------------------------------------------------------------------
//
// SFileMgr
//
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include <ios>
#include <iostream>
#include <vector>
#include <fstream>
#include "Standard/Path.h"
#include "Standard/Filesystem.h"

class PathFileMgr;

// FileMethod
class STANDARD_API FileMethod
{
public:
	virtual ~FileMethod() {}

	virtual std::iostream* open(const Path& fName, std::ios::openmode mode) = 0;
	virtual bool validFor(const Path& fName) = 0;
};

class STANDARD_API FileMethodDiskRelative : public FileMethod
{
public:
	FileMethodDiskRelative(const Path& base);

	virtual std::iostream* open(const Path& fName, std::ios::openmode mode);
	virtual bool validFor(const Path& fName);

private:
	Path m_base;
};

// A filesystem composed of other filesystems.
// When a file referred to by a path is requested open, the first filesystem that indicates it can understand the path is used to open the file.
class STANDARD_API FilesystemCompound : public FilesystemImmutable
{
public:
	// normal constructor
	FilesystemCompound();

	// lazy people constructor - pass an array of FileMethod pointers with which to init the file mgr
	FilesystemCompound(FileMethod** pMethods);

	virtual ~FilesystemCompound() {}

	void addFileMethod(FileMethod* pMethod);
	const Path& lastOpen() const { return m_lastOpen; }

	virtual Path absolute(const Path& path) const;
	virtual bool directory(const Path& path) const;
	virtual bool exists(const Path& path) const;
	virtual std::iostream* open(const Path& path, std::ios::openmode mode) const;
	virtual size_t size(const Path& path) const;

protected:
	mutable Path m_lastOpen; // tbd: fix filesystem class const correctness
	std::vector<SmartPtr<FileMethod> > m_fileMethods;
};

////
