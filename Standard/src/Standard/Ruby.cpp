#include "Standard/pch.h"
#ifdef STANDARD_ENABLE_RUBY

#include "Ruby.h"
#include "Standard/CommandLine.h"
#include "Standard/DebugInfo.h"
#include "Standard/Exception.h"
#include "Standard/Log.h"
#include "Standard/Path.h"

#if IS_GNUC
	#include <unistd.h>
#else
	#include <conio.h>
	#include <direct.h>
	#include <shellapi.h>
	#include <winsock2.h>
#endif

extern "C"
{
#if IS_WINDOWS
	#include "c:/ruby/lib/ruby/1.8/i386-mswin32/ruby.h"
	//#include <ruby.h>
#else
	#include <ruby.h>
#endif

	#undef write
	#undef getcwd
}

#include <iostream>

#if IS_MSVC && !NO_RUBY
	//#pragma comment(lib, "msvcr71-ruby18.lib")
	#pragma comment(lib, "msvcrt-ruby18.lib")
#endif

Ruby* Ruby::m_instance = 0;

////

class RubyLogStream : public dlogstream
{
public:
	RubyLogStream(Ruby& ruby, int fd)
	{
		if (!rbClass)
		{
			rbClass = rb_define_class("RubyLogStream", rb_cIO);
			rb_define_method(rbClass, "write", RUBY_METHOD_FUNC(&RubyLogStream::rbWrite), 1);
		}

		rbObj = rb_funcall(rbClass, rb_intern("new"), 2, INT2FIX(fd), rb_str_new2("w"));
		rb_iv_set(rbObj, "@cppinstance", INT2NUM((long)this));
	}

	static VALUE rbWrite(VALUE rbSelf, VALUE argStr)
	{
		RubyLogStream& self = *(RubyLogStream*)NUM2INT(rb_iv_get(rbSelf, "@cppinstance"));

		long len = 0;
		char* output = rb_str2cstr(argStr, &len);
		self.s().write(output, len);

		if (output[0] == 10)
			self.s().flush();

		return Qnil;
	}

	VALUE rbObj;
	static VALUE rbClass;
};

VALUE RubyLogStream::rbClass = 0;

////

void STANDARD_API _ruby_backtrace()
{
	Ruby::interpreter().backtrace();
}

#if IS_WINDOWS
void Ruby::start(LPSTR lpCmdLine, const char* loadFileName, const char* scriptName, bool gems)
{
	int argc = 0;
	char** argv = 0;

	NtInitialize(&argc, &argv);

	//AllocConsole();

	//dlog.attachToConsole();
	//dwarn.attachToConsole();
	//derr.attachToConsole();
	//ddbg.attachToConsole();

//	std::ifstream* console_in = new std::ifstream("CONIN$");
//	std::cin.rdbuf(console_in->rdbuf());

//	std::ofstream* console_err = new std::ofstream("CONOUT$");
//	std::cerr.rdbuf(console_err->rdbuf());

	Ruby::start(argc, (const char**)argv, loadFileName, scriptName, gems);
}
#endif

void Ruby::start(int argc, const char** argv, const char* loadFileName, const char* scriptName, bool gems)
{
	new Ruby(argc, argv, loadFileName, scriptName, gems);
}

void Ruby::destroy()
{
	ruby_finalize();

	delete m_logStreamStdOut;
	m_logStreamStdOut = 0;

	delete m_logStreamStdErr;
	m_logStreamStdErr = 0;

	delete m_instance;
	m_instance = 0;
}

static void rubyAtExit()
{
	// tbd: add check for ruby_options
	//throwf("Fatal error: ruby called 'exit'!\n\nCallstack:\n%s", DebugInfo::callstackStr().c_str());
}

Ruby::Ruby(int argc, const char** argv, const char* loadFileName, const char* scriptName, bool gems) :
	m_logStreamStdOut(0),
	m_logStreamStdErr(0)
{
	CommandLine cmdLine(argc, argv);

	m_instance = this;

	// add an atexit function in case ruby wants to call 'exit'
	atexit(&rubyAtExit);

	ruby_init();

	redirectOutput();

	if (argc < 1)
		throwf("argv[0] must be the full path to the application.");

	eval("puts 'Initializing Embedded Ruby...'");
	eval("puts");

	eval("puts 'Commandline:'");
	for (uint i = 0; i < cmdLine.size(); ++i)
		eval("puts '" + cmdLine[i] + "'");
	eval("puts");

	Path exePath = Path(cmdLine[0]).absolute();

	// find root of ruby library by walking parent directories
	Path prefix = exePath.dirPath();
	while (true)
	{
		Path searchPath = prefix + Path("embedded_ruby");

		eval(std::string("puts \"Searching for ruby library in '") + searchPath.forwardSlashes() + std::string("'...\""));

		if (searchPath.exists())
		{
			prefix = searchPath;
			break;
		}

		Path nextPrefix = prefix.parent();
		if (nextPrefix == prefix || nextPrefix.empty())
		{
			throwf("Couldn't find the library directory 'embedded_ruby' in any parent directory.");
		}

		prefix = nextPrefix;
	}

	eval(std::string("$embedded_prefix='") + prefix.forwardSlashes() + std::string("'"));
	eval("puts \"Locating embedded ruby library at #{$embedded_prefix}.\"");
	eval("puts");

	// setup loadpath
	// add project root to loadpath
	eval(std::string("$: << '") + prefix.parent().forwardSlashes() + std::string("'"));

	// add libroot to loadPath
	std::string prefixStr = prefix.forwardSlashes();

	eval(std::string("$: << '") + prefixStr + std::string("'"));

	eval(std::string("$: << '") + prefixStr + std::string("/lib/embedded'"));

#if STANDARD_RUBY_NO_RELATIVE_LIB_DEFS
	// RUBY_ path defs are not relative to the ruby dll on apple's ruby for osx as with i.e. windows, but are absolute paths to system Library location.
	eval(std::string("$: << '") + prefixStr + std::string("/lib/ruby/1.8'"));
	eval(std::string("$: << '") + prefixStr + std::string("/lib/ruby/1.8/") + std::string(RUBY_PLATFORM) + std::string("'"));
	eval(std::string("$: << '") + prefixStr + std::string("/lib/ruby/site_ruby/1.8'"));
	eval(std::string("$: << '") + prefixStr + std::string("/lib/ruby/site_ruby/1.8/") + std::string(RUBY_PLATFORM) + std::string("'"));
#else
	eval(std::string("$: << '") + prefixStr + std::string(RUBY_SITE_LIB2) + std::string("'"));
	eval(std::string("$: << '") + prefixStr + std::string(RUBY_SITE_ARCHLIB) + std::string("'"));
	eval(std::string("$: << '") + prefixStr + std::string(RUBY_SITE_LIB) + std::string("'"));
	eval(std::string("$: << '") + prefixStr + std::string(RUBY_LIB) + std::string("'"));
	eval(std::string("$: << '") + prefixStr + std::string(RUBY_ARCHLIB) + std::string("'"));
#endif

	eval(std::string("$: << '") + prefixStr + std::string("/lib'"));

	eval("puts 'Loadpath:'");
	eval("puts $:.join('\n')");
	eval("puts");

	// setup args
	VALUE args = rb_ary_new();
	int argStart = 1;

	// execute rbconfig with overrides for embedded ruby
	eval("puts 'Require rbconfig...'");
	eval("require 'rbconfig'");
	eval("puts \"Prefix is #{Config::CONFIG['prefix']}\"");
	eval("puts \"Sitedir is #{Config::CONFIG['sitedir']}\"");

	eval(std::string("Config::CONFIG['bindir'] = '") + *exePath.dirPath().absolute() + std::string("'"));
	eval("puts \"bindir is #{Config::CONFIG['bindir']}\"");
	eval(std::string("Config::CONFIG['EXEEXT'] = '") + *exePath.extension() + std::string("'"));
	eval("puts \"EXEEXT is #{Config::CONFIG['EXEEXT']}\"");
	eval(std::string("Config::CONFIG['ruby_install_name'] = '") + *exePath.fileNameNoExt() + std::string("'"));
	eval("puts \"ruby_install_name is #{Config::CONFIG['ruby_install_name']}\"");
	eval(std::string("Config::CONFIG['RUBY_INSTALL_NAME'] = '") + *exePath.fileNameNoExt() + std::string("'"));
	eval("puts \"RUBY_INSTALL_NAME is #{Config::CONFIG['RUBY_INSTALL_NAME']}\"");

	eval("puts");

	// execute script
	bool scriptExecMode = false;
	bool firstArgIsRb = false;
	bool firstArgIsBinProgram = false;
	bool debugging = false;
	bool ide = false;

	// check for ide mode
	if (cmdLine.has("ide"))
	{
		ide = true;
		cmdLine.removeArg("ide");
	}
	// check for debug mode
	else if (cmdLine.has("debug"))
	{
		debugging = true;
		cmdLine.removeArg("debug");
	}
	// check if a ruby script was given as the first option
	else if (cmdLine[1].find(".rb") != std::string::npos)
	{
		firstArgIsRb = true;
	}
	else
	{
		Path argv1 = (this->prefix() + Path("bin") + Path(cmdLine[1]));
		if (argv1.exists() && !argv1.isDirectory())
		{
			eval("puts 'Found a program matching argv[1] in embedded bin directory, executing...'");
			eval("Dir.chdir(File.join(Config::CONFIG['prefix'], 'bin'))");
			eval("puts \"Dir.pwd is #{Dir.pwd}\"\n");
			firstArgIsBinProgram = true;
		}
	}

	// check if a program in the 'bin' directory was given
	scriptExecMode = firstArgIsRb || firstArgIsBinProgram;

	// setup ARGV
	if (scriptExecMode)
	{
		argStart = 2;
	}
	else
	{
		argStart = 1;
	}

	for (int i = argStart; i < argc; ++i)
	{
		rb_ary_push(args, rb_str_new2(argv[i]));
	}

	// commandline script execution
	if (scriptExecMode)
	{
		// ./ is expected for $0 == __FILE__ comparisons
		if (cmdLine[1].find("./") == std::string::npos)
		{
			cmdLine[1] = std::string("./") + cmdLine[1];
		}

		eval(std::string("puts 'Preparing ruby script ") + std::string(cmdLine[1]) + std::string("...'"));

		CommandLine interpArgs;

		interpArgs.append(cmdLine[1]);
		interpArgs.append(cmdLine[1]);

		for (uint i = 2; i < cmdLine.size(); ++i)
			interpArgs.append(cmdLine[i]);

		ruby_options(interpArgs.argc(), (char**)interpArgs.argv());
		// Execution ends after function call.
		ruby_run();
	}

	if (ide)
	{
		eval("puts 'Interpreter invoked from ide, passing args to ruby...'");

		CommandLine interpArgs(cmdLine);
		cmdLine.removeArg("ide");

		eval("puts 'Calling ruby_options...'");

		ruby_options(interpArgs.argc(), (char**)interpArgs.argv());
	}
	else
	{
		rb_funcall(eval("ARGV"), rb_intern("replace"), 1, args);

		eval("puts 'ARGV:'");
		eval("puts ARGV.join(' ')");
		eval("puts");

		ruby_script(cmdLine[0].c_str());
	}

	if (gems)
	{
		eval("puts 'Rubygems active.'");
		eval("puts");
		eval("require 'ubygems'");
	}

	if (debugging)
	{
		eval("puts 'Debugger active, initialising...'");
		eval("require 'ruby-debug'");
		//eval("Debugger.main('localhost', 1234)");
	}

	if (ide)
	{
		eval("load $0");
	}
	else if (loadFileName)
	{
		eval(std::string("puts 'Running ") + std::string(loadFileName) + std::string("...'"));
		std::string loadCmd = std::string("load '") + loadFileName + std::string("'");
		eval(loadCmd.c_str());
	}
}

void Ruby::backtrace()
{
	eval("puts caller");
}

Ruby& Ruby::interpreter()
{
	if (!m_instance)
		throwf("Ruby interpreter is not initialized.");

	return *m_instance;
}

void Ruby::load(const char* fileName)
{
	rb_load_file(fileName);
}

void Ruby::startDebugger()
{
	eval("require 'ruby-debug'");
}

VALUE Ruby::eval(const char* string)
{
	int p = 0;
    VALUE result = rb_eval_string_protect(string, &p);
	if (p != 0)
		throw(ExceptionRuby());

	return result;
}

const char* Ruby::eval_to_s(const char* string)
{
	return STR2CSTR(rb_obj_as_string(eval(string)));
}

void Ruby::redirectOutput()
{
	rb_rescue(RUBY_METHOD_FUNC(&Ruby::rbRedirectOutput), (VALUE)this, RUBY_METHOD_FUNC(&Ruby::rbRescue), Qnil);
}

VALUE Ruby::rbRedirectOutput(VALUE rbSelf)
{
#if IS_WINDOWS
	Ruby& self = *(Ruby*)rbSelf;

	assert(!self.m_logStreamStdOut && !self.m_logStreamStdErr);

	self.m_logStreamStdOut = new RubyLogStream(self, 1);
	self.m_logStreamStdOut->add(new dlogprinterdebug);
	self.m_logStreamStdOut->add(new dlogprinterstdout);
	rb_gv_set("$embedded_stdout", self.m_logStreamStdOut->rbObj);
	rb_gv_set("$stdout", self.m_logStreamStdOut->rbObj);
	self.eval("STDOUT.reopen($embedded_stdout)");
	self.eval("STDOUT.instance_variable_set('@cppinstance',$embedded_stdout.instance_variable_get('@cppinstance'))");

	self.m_logStreamStdErr = new RubyLogStream(self, 2);
	self.m_logStreamStdErr->add(new dlogprinterdebug);
	self.m_logStreamStdErr->add(new dlogprinterstderr);
	rb_gv_set("$embedded_stderr", self.m_logStreamStdErr->rbObj);
	rb_gv_set("$stderr", self.m_logStreamStdOut->rbObj);
	self.eval("STDERR.reopen($embedded_stderr)");
	self.eval("STDERR.instance_variable_set('@cppinstance',$embedded_stderr.instance_variable_get('@cppinstance'))");

	//self.m_logStreamStdOut->attachToConsole();
	//self.m_logStreamStdErr->attachToConsole();

	/*HANDLE hIn = CreateFile("CONIN$", GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hIn == INVALID_HANDLE_VALUE)
		throwf("Couldn't open console stdin.");

	if (!SetStdHandle(STD_INPUT_HANDLE, hIn))
		throwf("Couldn't redirect stdin.");*/

	//rb_gv_set("$stdin", INT2FIX(hIn));
	//self.eval("$stdin = IO.new($stdin)");
	//self.eval("STDIN.reopen($stdin)");

#endif
	return Qnil;
}

VALUE Ruby::rbRescue()
{
	throw(ExceptionRuby());
}

///

ExceptionRuby::ExceptionRuby() :
	ExceptionString("")
{
	message = "Ruby exception:\n";

	try
	{
		VALUE exception = rb_gv_get("$!");
		char* buffer = RSTRING(rb_obj_as_string(exception))->ptr;
		message += buffer;
		message += "\n";
		message += STR2CSTR(rb_funcall(rb_funcall(exception, rb_intern("backtrace"), 0), rb_intern("join"), 1, rb_str_new2("\n")));

		std::cout << message.c_str() << std::endl;

#if IS_WINDOWS
		if (IsDebuggerPresent())
		{
			OutputDebugString(message.c_str());
		}
#endif
	}
	catch(...)
	{
		message += "Couldn't retrieve exception info.";

		try
		{
			std::cout << message.c_str() << std::endl;
#if IS_WINDOWS
			if (IsDebuggerPresent())
			{
				OutputDebugString(message.c_str());
			}
#endif
		}
		catch(...)
		{
		}
	}
}

Path Ruby::bindir()
{
	return Path(eval_to_s("Config::CONFIG['bindir']"));
}

Path Ruby::prefix()
{
	return Path(eval_to_s("Config::CONFIG['prefix']"));
}

#endif
