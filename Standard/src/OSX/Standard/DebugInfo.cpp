// ------------------------------------------------------------------------------------------------
//
// DebugInfo
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "DebugInfo.h"

#if !IS_WINDOWS
	#include <execinfo.h>
#endif

namespace DebugInfo
{
#if IS_WINDOWS
	STANDARD_API std::vector<std::string> callstack(PCONTEXT context, void* startAddr)
	{
		unsigned long _ip;
		unsigned long _frame;
		unsigned long _stack;
		__asm
		{
			mov eax, [ebp+8]
			mov _ip, eax
			mov _frame, ebp
			mov _stack, esp
		};

		if (context)
		{
			_ip = context->Eip;
			_frame = context->Ebp;
			_stack = context->Esp;
		}

		std::vector<std::string> v;

		HMODULE dll = LoadLibrary("dbghelp.dll");
		if (!dll)
			return v;

		LPAPI_VERSION version = ImagehlpApiVersion();

		HANDLE h = GetCurrentProcess();

		if (!SymInitialize(h, 0, _MAX_PATH))
			return v;

/*		char path[_MAX_PATH];
		if (!(*symSearchPath)(GetCurrentProcess(), path, _MAX_PATH))
			return v;*/

		STACKFRAME64 s;
		zero(s);
		s.AddrPC.Offset = _ip;
		s.AddrPC.Mode = AddrModeFlat;
		s.AddrFrame.Offset = _frame;
		s.AddrFrame.Mode = AddrModeFlat;
		s.AddrStack.Offset = _stack;
		s.AddrStack.Mode = AddrModeFlat;

		ULONG64 startSymAddr;
		bool bFoundAddr;

		if (startAddr)
		{
			char buffer[512];
			PSYMBOL_INFO info = (PSYMBOL_INFO)buffer;
			zero(buffer);
			info->SizeOfStruct = sizeof(SYMBOL_INFO);
			info->MaxNameLen = sizeof(buffer) - sizeof(SYMBOL_INFO) + 1;

			bFoundAddr = !SymFromAddr(h, (DWORD64)startAddr, 0, info);

			startSymAddr = info->Address;
		}
		else
		{
			bFoundAddr = true;
		}


		// ignore first stack entry -- it's the handler function
		while (StackWalk64(IMAGE_FILE_MACHINE_I386, h, 0, &s, 0, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
		{
			char buffer[512];
			PSYMBOL_INFO info = (PSYMBOL_INFO)buffer;

			zero(buffer);
			info->SizeOfStruct = sizeof(SYMBOL_INFO);
			info->MaxNameLen = sizeof(buffer) - sizeof(SYMBOL_INFO) + 1;

			if (SymFromAddr(h, s.AddrPC.Offset, 0, info))
			{
				if (!bFoundAddr && startSymAddr == info->Address)
					bFoundAddr = true;

				if (bFoundAddr)
					v.push_back(info->Name);
			}
			else
				v.push_back("<no symbol " + ptostr((void*)s.AddrPC.Offset) + ">");
		}

		SymCleanup(h);

		return v;
	}
#else
	std::vector<std::string> callstack()
	{
		const int max = 512;
		void* addresses[max];

		backtrace(addresses, max);

		char** symbols = backtrace_symbols(addresses, max);

		std::vector<std::string> result;

		for (int i = 0; i < max; ++i)
		{
			if (!symbols[i][0])
				break;

			result.push_back(symbols[i]);
		}

		free(symbols);

		return result;
	}
#endif
}
