#include "Standard/ExceptionPosix.h"
#include <errno.h>
#include <string.h>

ExceptionPosix::ExceptionPosix()
{
	addContext("Posix system call error ");
	_context << "(errno " << errno << ": ";
	_context << strerror(errno);
	_context << ")";
}
