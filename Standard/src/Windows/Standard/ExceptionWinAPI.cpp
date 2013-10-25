#include "Standard/pch.h"
#include "ExceptionWinAPI.h"

ExceptionWinAPI::ExceptionWinAPI(const std::string& context)
{
#if _UNICODE
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, m_buffer, 1024, 0);
#else
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, m_buffer, 1024, 0);
#endif
	m_result += m_buffer;
	m_result += " (";
	m_result += context;
	m_result += ")";
}

const char* ExceptionWinAPI::what() const throw()
{
	return m_result.c_str();
}
