#include "Standard/api.h"
#include "Standard/CommandLine.h"

#include <iostream>

#define EMBEDDED_RUBY 0

#if EMBEDDED_RUBY
#include "Standard/Ruby.h"

int main(int argc, char* argv[])
{
	//setenv("DYLD_FRAMEWORK_PATH", (*(Path::exePath().dirPath() + Path("../Frameworks")).full()).c_str(), true);
	//setenv("DYLD_FALLBACK_FRAMEWORK_PATH", (*(Path::exePath().dirPath() + Path("../Frameworks")).full()).c_str(), true);
	//setenv("DYLD_PRINT_ENV", "1", true);
	
	try
	{
		Ruby::start(argc, (const char**)argv, "dj.rb", "dj");
	}
	catch (ExceptionRuby&)
	{
		printf("Ruby exception, program ending...\n");
	}
	catch (ExceptionString& e)
	{
		std::cout << e.message.c_str() << std::endl;
	};
	
	std::cout << "Program complete." << std::endl;
	//std::cin.get();
	
	return 0;
}

#else
int main(int argc, char* argv[])
{
	CommandLine cmdLine(argc, argv);
	
	cmdLine.prepend("dj.rb");
	cmdLine.prepend("ruby");
	
	const char* const* newArgv = cmdLine.argv();
	
	execvp(newArgv[0], newArgv); 
	
	return 0;
}
#endif