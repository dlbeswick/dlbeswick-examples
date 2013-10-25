// ------------------------------------------------------------------------------------------------
//
// Path
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Path.h"
#include "Filesystem.h"
#include "Help.h"
#include "Exception.h"
#include "STLHelp.h"
#include <algorithm>
#include <string>

const std::string Path::SEPARATORS = "\\/";

#if IS_WINDOWS
	const char Path::PLATFORM_SEPARATOR = '\\';
#else
	const char Path::PLATFORM_SEPARATOR = '/';
#endif

Path::Path(const std::string& stringRep) :
	pathRep(stringRep),
	_filesystem(&FilesystemOS())
{
	format(pathRep);
}

Path::Path(const std::string& stringRep, Filesystem& filesystem) :
	pathRep(stringRep),
	_filesystem(&filesystem)
{
	format(pathRep);
}

Path::Path(const Path& rhs) :
	pathRep(rhs.pathRep),
	_filesystem(rhs._filesystem)
{
}

void Path::operator = (const Path& rhs)
{
	pathRep = rhs.pathRep;
	_filesystem = rhs._filesystem;
}

Path Path::operator + (const Path& rhs) const { Path p(*this); p += rhs; return p; }
void Path::operator += (const Path& rhs) { append(rhs); }
const std::string& Path::operator * () const { return pathRep; }

bool Path::operator == (const Path& rhs) const
{
	return pathRep == rhs.pathRep;
}

bool Path::isSeparator(char c)
{
	return std::find_first_of(&c, (&c)+1, SEPARATORS.begin(), SEPARATORS.end()) != (&c)+1;
}

Path Path::dirPath() const
{
	if (directory()
		|| (!exists() && extension().empty()))
		return *this;

	uint lastSepIdx = pathRep.rfind(PLATFORM_SEPARATOR);

	if (lastSepIdx == std::string::npos)
	{
		return *this;
	}
	else
	{
		return Path(pathRep.substr(0, lastSepIdx));
	}
}

Path Path::fileName() const
{
	uint lastSepIdx = pathRep.rfind(PLATFORM_SEPARATOR);

	if (lastSepIdx == std::string::npos)
	{
		return Path();
	}
	else
	{
		std::string last = pathRep.substr(lastSepIdx + 1);

		uint dotIdx = last.rfind('.');

		if (dotIdx == std::string::npos)
			return Path();

		return Path(last);
	}
}

Path Path::fileNameNoExt() const
{
	uint lastSepIdx = pathRep.rfind(PLATFORM_SEPARATOR);

	if (lastSepIdx == std::string::npos)
	{
		return *this;
	}
	else
	{
		return Path(pathRep.substr(lastSepIdx + 1, pathRep.rfind('.') - lastSepIdx));
	}
}

Path Path::extension() const
{
	std::string fName = *fileName();
	uint lastExtIdx = fName.rfind('.');

	if (lastExtIdx == std::string::npos)
	{
		return Path("");
	}
	else
	{
		return Path(fName.substr(lastExtIdx + 1));
	}
}

Path Path::absolute() const
{
	return _filesystem->absolute(*this);
}

bool Path::exists() const
{
	return _filesystem->exists(*this);
}

size_t Path::size() const
{
	return _filesystem->size(*this);
}

void Path::del() const
{
	_filesystem->del(*this);
}

void Path::format(std::string& s) const
{
	std::replace_if(s.begin(), s.end(), isSeparator, PLATFORM_SEPARATOR);

	// chop trailing slash
	if (!s.empty())
	{
		s.erase(std::remove_if(--s.end(), s.end(), isSeparator), s.end());
	}
}

std::string Path::forwardSlashes() const
{
	std::string newPath = pathRep;

	std::replace_if(newPath.begin(), newPath.end(), isSeparator, '/');

	return newPath;
}

void Path::chop()
{
	if (!pathRep.empty())
	{
		pathRep.erase(std::remove_if(--pathRep.end(), pathRep.end(), isSeparator), pathRep.end());
	}
}

void Path::append(const Path& rhs)
{
	if (rhs.empty())
		return;

	if (!empty())
	{
		pathRep += PLATFORM_SEPARATOR;
	}

	if (rhs.pathRep[0] == PLATFORM_SEPARATOR)
		pathRep += rhs.pathRep.substr(1);
	else
		pathRep += rhs.pathRep;

	chop();
}

size_t Path::segments() const
{
	if (empty())
		return 0;

	return std::count(pathRep.begin(), pathRep.end(), PLATFORM_SEPARATOR) + 1;
}

std::string Path::operator[](size_t idx) const
{
	size_t startIdx = 0;
	size_t endIdx = std::string::npos;

	for (uint i = 0; ; ++i)
	{
		endIdx = pathRep.find(PLATFORM_SEPARATOR, startIdx+1);
		if (endIdx == std::string::npos)
			break;

		if (i == idx)
			break;

		startIdx = endIdx;
	}

	return pathRep.substr(startIdx, (endIdx - startIdx));
}

bool Path::directory() const
{
	return _filesystem->directory(*this);
}

void Path::create() const
{
	_filesystem->create(*this);
}

bool Path::readOnly() const
{
	return _filesystem->readOnly(*this);
}

// returns the path's parent
Path Path::parent() const
{
	std::string opPathRep;

	if (!fileName().empty())
		opPathRep = dirPath().pathRep;
	else
		opPathRep = pathRep;

	uint lastSepIdx = opPathRep.rfind(PLATFORM_SEPARATOR);

	if (lastSepIdx == std::string::npos)
	{
		return *this;
	}
	else
	{
		return Path(opPathRep.substr(0, lastSepIdx));
	}
}

// returns the local home settings path
Path Path::localHome(const Path& p)
{
#if IS_WINDOWS
	return Path("local") + p;
#else
	return Path("~") + p;
#endif
}

Path Path::localSettings(const Path& p)
{
#if IS_WINDOWS
	return localHome(p);
#else
	return localHome(Path("Documents") + p);
#endif
}

Path Path::exePath()
{
#if IS_OSX
//	CFBundleRef mainBundle = CFBundleGetMainBundle();
//	CFURLRef exePath = CFBundleCopyExecutableURL(mainBundle);
//
//	if (!exePath)
//		throwf("No executable found.");
//
//	UInt8 fsRep[PATH_MAX];
//	CFURLGetFileSystemRepresentation(exePath, true, fsRep, PATH_MAX);
//
//	return Path((char*)fsRep);
	return Path();
#elif IS_LINUX
	portRequired("Path::exePath");
#else
	char fsRepA[PATH_MAX] = {0};
	int result = GetModuleFileName(0, fsRepA, PATH_MAX);

	if (result == 0)
	{
		throwf("Couldn't get exe path.");
	}

	return Path(fsRepA);
#endif
}

bool Path::isAbsolute() const
{
	return absolute() == *this;
}

std::iostream* Path::open(std::ios::openmode mode) const
{
	// open for reading by default if neither in nor out is specified
	if (!((mode & std::ios::in) || (mode & std::ios::out)))
		mode |= std::ios::in;

	return _filesystem->open(*this, mode);
}
