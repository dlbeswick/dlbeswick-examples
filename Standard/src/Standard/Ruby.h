#ifndef STANDARD_RUBY_H
#define STANDARD_RUBY_H
#ifndef STANDARD_ENABLE_RUBY

#include "Standard/api.h"
#include "Standard/Exception.h"
#include "Standard/Path.h"

// An embedded ruby interpreter.
class STANDARD_API Ruby
{
public:
	typedef unsigned long VALUE;

#if IS_WINDOWS
	static void start(LPSTR lpCmdLine, const char* loadFileName, const char* scriptName = "EmbeddedRuby", bool gems = true);
#endif

	static void start(int argc, const char** argv, const char* loadFileName, const char* scriptName = "EmbeddedRuby", bool gems = true);
	void destroy();

	virtual void backtrace();

	virtual VALUE eval(const std::string& string) { return eval(string.c_str()); }
	virtual VALUE eval(const char* string);
	virtual const char* eval_to_s(const char* string);
	virtual const char* eval_to_s(const std::string& string) { return eval_to_s(string.c_str()); }

	virtual void load(const char* fileName);
	virtual void startDebugger();

	static Ruby& interpreter();

	virtual Path bindir();
	virtual Path prefix();

protected:
	Ruby(int argc, const char** argv, const char* loadFileName, const char* scriptName = "EmbeddedRuby", bool gems = true);

	virtual void redirectOutput();
	static VALUE rbRedirectOutput(VALUE rbSelf);

	static VALUE rbRescue();

	static Ruby* m_instance;
	class RubyLogStream* m_logStreamStdOut;
	class RubyLogStream* m_logStreamStdErr;
};

class STANDARD_API ExceptionRuby : public ExceptionString
{
public:
	ExceptionRuby();
};

#endif
#endif
