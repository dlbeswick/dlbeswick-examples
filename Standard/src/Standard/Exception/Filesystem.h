#ifndef STANDARD_EXCEPTION_FILESYSTEM_H
#define STANDARD_EXCEPTION_FILESYSTEM_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"
#include "Standard/Path.h"

class STANDARD_API ExceptionFilesystem : public ExceptionStream
{
public:
	ExceptionFilesystem(const Path& _path, const std::string& context = "");
	virtual ~ExceptionFilesystem() throw() {};

	Path path;
};

#endif
