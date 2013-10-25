// ---------------------------------------------------------------------------------------------------------
// 
// FileHelp
// obsolete
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#if 0


#include "Standard/api.h"
#include "STLHelp.h"
#include "Exception.h"
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <direct.h>

namespace FileHelp
{
	// returns the separator characters
	inline std::string separators()
	{
		return std::string("\\/");
	}

	// returns true if the given character is a path seperator
	inline bool isSeparator(char c)
	{
		return c == '\\' || c == '/';
	}

	// minus the filename from a full path
	inline std::string path(const std::string& inPath)
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];

		char fullPath[_MAX_PATH];

		_fullpath(fullPath, inPath.c_str(), _MAX_PATH);
		_splitpath(fullPath, drive, dir, 0, 0);

		return drive + std::string(dir);
	}

	// get the filename from a full path
	inline std::string fileName(const std::string& inPath)
	{
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char fullPath[_MAX_PATH];

		_fullpath(fullPath, inPath.c_str(), _MAX_PATH);
		_splitpath(fullPath, 0, 0, fname, ext);

		return std::string(fname) + ext;
	}

	// expands a relative path
	inline std::string fullPath(const std::string& inPath)
	{
		char fullPath[_MAX_PATH];

		_fullpath(fullPath, inPath.c_str(), _MAX_PATH);

		return fullPath;
	}

	// adds a slash to the end of a path if there's not one there, else just appends the string
	inline std::string appendedPath(const std::string& inPath, const std::string& append)
	{
		std::string ret = inPath;

		if (!inPath.empty() && !isSeparator(inPath[inPath.size() - 1]))
			ret += "\\";
		
		if (!append.empty() && !isSeparator(append[0]))
			ret += append.substr(0);
		else
			ret += append;

		return ret;
	}

	// returns true if the file or directory exists
	inline bool exists(const std::string& path)
	{
		return _access(path.c_str(), 0) != -1;
	}

	// gets the size of a file
	inline __int64 size(const std::string& path)
	{
		struct __stat64 buf;
		int result;

		result = _stat64(path.c_str(), &buf);
		if (result != 0)
			throwf("Couldn't get stats for file " + path);

		return buf.st_size;
	}

	// creates the given path if it doesn't exist
	// handles the creation of multiple folders if need be
	inline void createPath(const Path& path)
	{
		std::string cumulativePath;
		std::vector<std::string> dirs;

		split(*path, dirs, separators());

		for (uint i = 0; i < dirs.size(); i++)
		{
			if (!cumulativePath.empty())
				cumulativePath += "\\";

			cumulativePath += dirs[i];
			
			if (!exists(cumulativePath))
			{
				if (_mkdir(cumulativePath.c_str()) == -1)
					throwf("Couldn't create directory " + dirs[i] + " while creating path " + path);
			}
		}
	}

	// delete a file
	inline bool del(const std::string& path)
	{
		return remove(path.c_str()) != -1;
	}
};

#endif