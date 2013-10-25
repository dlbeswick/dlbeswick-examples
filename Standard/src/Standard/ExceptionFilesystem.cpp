#include "Standard/pch.h"
#include "streamops.h"
#include "Exception/Filesystem.h"

ExceptionFilesystem::ExceptionFilesystem(const Path& _path, const std::string& context) :
	path(_path)
{
	exceptionstream(*this) << "Filesystem Exception: '" << *path << "': ";

	if (!context.empty())
	  addContext(context);

	try
	{
		if (path.empty())
			addContext("empty path");
		else
			addContext(*path.absolute());
	}
	catch (ExceptionFilesystem&)
	{
		addContext(*path);
		addContext("('absolute' not supported.)");
	}
}
