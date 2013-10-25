#include "Standard/pch.h"
#include "Filesystem.h"
#include "Exception/Filesystem.h"
#include "Path.h"
#include "streamops.h"

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#if IS_POSIX
	#include <unistd.h>
#else
	#include <direct.h>
	#include <io.h>

	extern HINSTANCE g_dllInstance;
#endif

#if IS_MSVC
	#define mkdir _mkdir
	#define O_WRONLY _O_WRONLY

//	typedef __stat64 stat;

	#define access _access
	#define fullpath _fullpath
	#define S_IFDIR _S_IFDIR

	int stat(const char* path, struct __stat64* buffer) { return _stat64(path, buffer); }
#endif

#include <fcntl.h>

void FilesystemImmutable::create(const Path& path) const
{
	throw(ExceptionFilesystem(path, "Function not supported for immutable filesystem."));
}

void FilesystemImmutable::del(const Path& path) const
{
	throw(ExceptionFilesystem(path, "Function not supported for immutable filesystem."));
}

FilesystemPosix filesystemPosix;

FilesystemPosix& FilesystemOS()
{
	return filesystemPosix;
}

Path FilesystemPosix::absolute(const Path& path) const
{
	char fullPath[PATH_MAX];

#if IS_WINDOWS
	_fullpath(fullPath, (*path).c_str(), PATH_MAX);
#else
	realpath((*path).c_str(), fullPath);
#endif

	return Path(fullPath);
}

void FilesystemPosix::create(const Path& path) const
{
	Path cumulativePath;
	Path base = path.dirPath();

	for (uint i = 0; i < base.segments(); ++i)
	{
		cumulativePath += base[i];

		if (!cumulativePath.exists())
		{
#if IS_WINDOWS
			if (mkdir((*cumulativePath).c_str()) == -1)
#else
			if (mkdir((*cumulativePath).c_str(), S_IRWXU) == -1)
#endif
				EXCEPTIONSTREAM(ExceptionFilesystem(path), "Couldn't create directory '" + *cumulativePath + "'");
		}
	}
}

void FilesystemPosix::del(const Path& path) const
{
	if (remove((*path).c_str()) != -1)
		EXCEPTIONSTREAM(ExceptionFilesystem(path), "Couldn't delete file.");
}

bool FilesystemPosix::directory(const Path& path) const
{
	struct stat buf;

	int result;

	result = stat((*path).c_str(), &buf);
	if (result != 0)
		return false;

	return (buf.st_mode & S_IFDIR) != 0;
}

bool FilesystemPosix::exists(const Path& path) const
{
	if (access((*path).c_str(), 0) != -1)
		return true;

	return false;
}

std::iostream* FilesystemPosix::open(const Path& path, std::ios::openmode mode) const
{
	std::fstream* stream = new std::fstream((*path).c_str(), mode);

	if (!stream->good())
		EXCEPTIONSTREAM(ExceptionFilesystem(path), "Couldn't open file.");

	return stream;
}

bool FilesystemPosix::readOnly(const Path& path) const
{
	if (!exists(path))
		return false;

	int fd = ::open((*path).c_str(), O_WRONLY);
	close(fd);

	return fd == -1;
}

size_t FilesystemPosix::size(const Path& path) const
{
	struct stat buf;

	int result;

	result = stat((*path).c_str(), &buf);
	if (result != 0)
		return ((size_t)-1);

	return buf.st_size;
}

