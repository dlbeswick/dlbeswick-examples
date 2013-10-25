#ifndef STANDARD_FILESYSTEM_H
#define STANDARD_FILESYSTEM_H

#include "Standard/api.h"
#include <fstream>

class Path;

/// Encapsulates the functionality of a filesystem.
class STANDARD_API Filesystem
{
public:
	/// Returns the absolute representation of the path
	virtual Path absolute(const Path& path) const = 0;

	/// Creates the given path if it doesn't exist.
	/// Creates the directory tree if necessary.
	virtual void create(const Path& path) const = 0;

	/// Deletes the file represented by the path.
	virtual void del(const Path& path) const = 0;

	/// Returns true if the path is a directory.
	virtual bool directory(const Path& path) const = 0;

	/// Returns true if the file or directory represented by the path exists.
	virtual bool exists(const Path& path) const = 0;

	/// Returns a valid iostream corresponding to the path.
	virtual std::iostream* open(const Path& path, std::ios::openmode mode) const = 0;

	/// Returns the size of the file represented by the path
	virtual size_t size(const Path& path) const = 0;

	/// Returns true if the file represented by the path exists and is read-only
	virtual bool readOnly(const Path& path) const = 0;
};

/// Encapsulates the functionality of a filesystem whose entries cannot be modified.
class STANDARD_API FilesystemImmutable : public Filesystem
{
public:
	// TBD: improve class design.
	// These methods should not exist for immutable filesystems.

	/// Creates the given path if it doesn't exist.
	/// Creates the directory tree if necessary.
	virtual void create(const Path& path) const;

	/// Deletes the file represented by the path.
	virtual void del(const Path& path) const;

	virtual bool readOnly(const Path& path) const { return true; }
};

class STANDARD_API FilesystemPosix : public Filesystem
{
public:
	virtual Path absolute(const Path& path) const;
	virtual void create(const Path& path) const;
	virtual void del(const Path& path) const;
	virtual bool directory(const Path& path) const;
	virtual bool exists(const Path& path) const;
	virtual std::iostream* open(const Path& path, std::ios::openmode mode) const;
	virtual bool readOnly(const Path& path) const;
	virtual size_t size(const Path& path) const;
};

extern STANDARD_API FilesystemPosix& FilesystemOS();

#endif
