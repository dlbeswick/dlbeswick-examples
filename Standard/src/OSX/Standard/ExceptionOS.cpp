#include "Standard/pch.h"
#include "ExceptionOS.h"

ExceptionOS::ExceptionOS(const std::string& line, OSStatus error)
{
	_message = std::string("OSError: ") + line + std::string(": ") + GetMacOSStatusErrorString(error) + std::string(": ") + GetMacOSStatusCommentString(error);
}

ExceptionOS::~ExceptionOS() throw()
{
}

const char* ExceptionOS::what() const throw()
{
	return _message.c_str();
}
