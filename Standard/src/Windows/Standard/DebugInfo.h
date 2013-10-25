// ------------------------------------------------------------------------------------------------
//
// DebugInfo
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Standard/STLHelp.h"

namespace DebugInfo
{
	STANDARD_API std::vector<std::string> callstack(PCONTEXT context = 0, void* startAddr = 0);

	inline std::string strCallstack()
	{
		return join(callstack(), " -> ");
	}
}
