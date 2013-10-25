// ------------------------------------------------------------------------------------------------
//
// DebugInfo
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/STLHelp.h"

namespace DebugInfo
{
#if IS_WINDOWS
	STANDARD_API std::vector<std::string> callstack(PCONTEXT context = 0, void* startAddr = 0);
#else
	STANDARD_API std::vector<std::string> callstack();
#endif

	inline std::string callstackStr()
	{
		return join(callstack(), "\r\n -> ");
	}
}
