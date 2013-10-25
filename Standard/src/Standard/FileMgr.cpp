// ---------------------------------------------------------------------------------------------------------
//
// FilesystemCompound
//
// ---------------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Exception/Filesystem.h"
#include "Standard/streamops.h"
#include "FileMgr.h"
#include "Filesystem.h"
#include "Help.h"

#ifndef IS_MSVC
#pragma warning(disable:4786)
#endif

////

FileMethodDiskRelative::FileMethodDiskRelative(const Path& base) :
	m_base(base)
{
	if (m_base.empty())
		throwf("You must specify a base directory for FileMethodDiskRelative");

	if (&m_base.filesystem() != &FilesystemOS())
		throw("Base path supplied to FileMethodDiskRelative must use the OS filesystem.");
}

std::iostream* FileMethodDiskRelative::open(const Path& fName, std::ios::openmode mode)
{
	return (m_base + fName).open(mode);
}

bool FileMethodDiskRelative::validFor(const Path& fName)
{
	return std::fstream((*(m_base + fName)).c_str(), std::ios::in).good();
}

////

FilesystemCompound::FilesystemCompound() :
	m_lastOpen("no file opened")
{
}

FilesystemCompound::FilesystemCompound(FileMethod** pMethods) :
	m_lastOpen("no file opened")
{
	for (uint i = 0; ; i++)
	{
		if (!pMethods[i])
			break;

		addFileMethod(pMethods[i]);
	}
}

void FilesystemCompound::addFileMethod(FileMethod* pMethod)
{
	m_fileMethods.insert(m_fileMethods.begin(), pMethod);
}


Path FilesystemCompound::absolute(const Path& path) const
{
	// fix: Path() is used because absolute is called from ExceptionFilesystem, causing stack overflow.
	EXCEPTIONSTREAM(ExceptionFilesystem(Path()), "Filesystem does not support 'absolute' method.");
	return Path();
}

bool FilesystemCompound::directory(const Path& path) const
{
	// fix: Path() is used because absolute is called from ExceptionFilesystem, causing stack overflow.
	EXCEPTIONSTREAM(ExceptionFilesystem(Path()), "Filesystem does not support 'absolute' method.");
	return false;
}

bool FilesystemCompound::exists(const Path& path) const
{
	// fix: Path() is used because absolute is called from ExceptionFilesystem, causing stack overflow.
	EXCEPTIONSTREAM(ExceptionFilesystem(Path()), "Filesystem does not support 'absolute' method.");
	return false;
}

std::iostream* FilesystemCompound::open(const Path& path, std::ios::openmode mode) const
{
	m_lastOpen = path;

	if (m_fileMethods.empty())
		throwf("FileMgr: no methods available for opening files");

	std::iostream* pS = 0;
	uint idx = 0;

	do
	{
		FileMethod& m = *m_fileMethods[idx++];
		if (!m.validFor(path))
			continue;

		pS = m.open(path, mode);
	}
	while (!pS && idx < m_fileMethods.size());

	if (!pS || pS->fail())
		EXCEPTIONSTREAM(ExceptionFilesystem(path), "Compound filesystem could not open file.");

	return pS;
}

size_t FilesystemCompound::size(const Path& path) const
{
	assert(0);
	return 0;
}
