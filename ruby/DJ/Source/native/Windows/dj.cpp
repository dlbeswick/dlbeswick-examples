#define _WIN32_WINNT 0x0400 
#include "windows.h"
#include <conio.h>
#include "Standard/api.h"
#include "Standard/Log.h"

#define EMBEDDED_RUBY 0

#if EMBEDDED_RUBY

#include <iostream>
#include "Standard/Ruby.h"

int main(int argc, const char* argv[])
{
	try
	{
		Ruby::start(argc, argv, "dj.rb", "dj");
	}
	catch (ExceptionRuby&)
	{
		printf("Ruby exception, program ending.");
	}
	catch (ExceptionString& e)
	{
		printf(e.message.c_str());
	};

	printf("Program complete.");

	//if (!IsDebuggerPresent())
	//	getch();

	return 0;
}

#else

#include "Standard/CommandLine.h"
#include "process.h"

int main(int argc, const char* argv[])
{
	CommandLine cmd(argc, argv);
	CommandLine newCmd;

	newCmd.append("ruby");
	newCmd.append("dj.rb");
	newCmd.insertSlice(1, cmd, 1, cmd.size());

	const char* const* newArgv = newCmd.argv();

	int result = _spawnvp(_P_NOWAIT, newArgv[0], newArgv);

	if (result == -1)
	{
		printf("Failed to launch ruby (errno %d)", errno);
		return 1;
	}
	
	if (IsDebuggerPresent())
	{
		DebugActiveProcess(result);
	}

	WaitForSingleObject((HANDLE)result, INFINITE);

	printf("Program complete.");

	if (IsDebuggerPresent())
	{
		getch();
	}

	return 0;
}

#endif