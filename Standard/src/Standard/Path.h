#pragma once

#include "Standard/api.h"
#include <fstream>

class Filesystem;

class STANDARD_API Path
{
public:
	// Constructs a path from the given string representation.
	// If no filesystem is specified, then the OS's standard filesystem is used.
	Path(const std::string& stringRep = "");
	Path(const std::string& stringRep, Filesystem& filesystem);
	Path(const Path& rhs);
	void operator = (const Path& rhs);

	Path operator + (const Path& rhs) const;
	void operator += (const Path& rhs);
	bool operator == (const Path& rhs) const;

	// Returns a representation suitable for passing to the os.
	const std::string& operator * () const;

	// returns true if the given character is a path seperator
	static bool isSeparator(char c);

	// returns the absolute representation of the path
	Path absolute() const;

	// creates the given path if it doesn't exist
	// handles the creation of multiple folders if need be
	void create() const;

	// deletes the file represented by the path.
	void del() const;

	// return true if the path is a directory
	bool directory() const;

	// returns the path minus the filename
	Path dirPath() const;

	// returns true if path is the empty string
	bool empty() const { return pathRep.empty(); }

	// returns the path to the current executable, the meaning of which may be platform-dependent.
	static Path exePath();

	// returns true if the file or directory represented by the path exists
	bool exists() const;

	// returns the extension of the path's filename
	Path extension() const;

	// returns the filename from the path
	Path fileName() const;

	// returns the filename from the path without extension
	Path fileNameNoExt() const;

	// returns the path representation with forward slashes in place of the platform separator.
	std::string forwardSlashes() const;

	// returns the size of the file represented by the path
	size_t size() const;

	// return true if the path is an absolute path
	bool isAbsolute() const;

	// returns the local home directory path
	static Path localHome(const Path& p = Path(""));

	// returns the local user 'settings' path
	static Path localSettings(const Path& p = Path(""));

	// returns the path's parent
	Path parent() const;

	// returns true if the file represented by the path exists and is read-only
	bool readOnly() const;

	// returns number of path segments
	size_t segments() const;

	// returns path segment at index
	std::string operator[](size_t idx) const;

	/// Returns a valid iostream corresponding to the path.
	/// Ownership is given to the caller.
	/// Throws ExceptionFilesystem.
	std::iostream* open(std::ios::openmode mode = std::ios::in | std::ios::binary) const;

	Filesystem& filesystem() const { return *_filesystem; }

protected:
	void format(std::string& s) const;
	void chop();
	void append(const Path& rhs);

	class Filesystem* _filesystem;
	std::string pathRep;
	static const std::string SEPARATORS;
	static const char PLATFORM_SEPARATOR;
};
