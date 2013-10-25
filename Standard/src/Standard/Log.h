#ifndef STANDARD_LOG_H
#define STANDARD_LOG_H

#include "Standard/api.h"
#include <fstream>
#include "textstream.h"
#include "streamops.h"

class Path;

// subclass and override this class, then add to dlogstream to use different printing methods.
class STANDARD_API dlogprinter : public std::binary_function<const char*, int, void>
{
public:
	virtual ~dlogprinter() {}
	virtual void operator()(const char* data, int len) = 0;
};

class STANDARD_API dlogprinterdebug : public dlogprinter
{
public:
	virtual void operator()(const char* data, int len);
};

class STANDARD_API dlogprinterfile : public dlogprinter
{
public:
	dlogprinterfile(const Path& fname);
	dlogprinterfile(std::ostream* _s);
	virtual ~dlogprinterfile();

	virtual void operator()(const char* data, int len);

private:
	std::ostream* s;
};

class STANDARD_API dlogprinterstdout : public dlogprinter
{
public:
	virtual void operator()(const char* data, int len);
};

class STANDARD_API dlogprinterstderr : public dlogprinter
{
public:
	virtual void operator()(const char* data, int len);
};

// dlog stream
class STANDARD_API dlogstream : public otextstream
{
	class internalstreambuf : public std::stringbuf
	{
	public:
		explicit internalstreambuf(std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
		explicit internalstreambuf(const std::string& str, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

		virtual ~internalstreambuf();

		void add(dlogprinter* p);

	protected:
		std::vector<dlogprinter*> m_printers;

		virtual int sync();
		dlogstream* m_stream;

		friend class dlogstream;
	};

	friend class internalstreambuf;

	class internalstream : public std::basic_ostream<char>
	{
	public:
		internalstream() : std::basic_ostream<char>(new internalstreambuf) {}
	};

	static class Timer timestamp;
	bool m_enabled;

protected:
	virtual bool shouldPrint();

	bool m_useTimestamp;

public:
	dlogstream(bool useTimestamp = true);
	virtual ~dlogstream();

	void add(dlogprinter* p);
	void setEnabled(bool b);

	void redirect(std::ostream& source);

#if IS_WINDOWS
	void attachToConsole();
#endif
};

class STANDARD_API dlogstreamstdout : public dlogstream
{
public:
	dlogstreamstdout();
};

class STANDARD_API dlogstreamstderr : public dlogstream
{
public:
	dlogstreamstderr();
};

#if !IS_GNUC
	extern STANDARD_API dlogstreamstdout dlog;
	extern STANDARD_API dlogstreamstdout dwarn;
	extern STANDARD_API dlogstreamstderr derr;
	extern STANDARD_API dlogstreamstdout ddbg;
#else
	extern dlogstreamstdout& dlog;
	extern dlogstreamstdout& dwarn;
	extern dlogstreamstderr& derr;
	extern dlogstreamstdout& ddbg;
	extern SmartPtr<dlogstreamstdout> dlog_ref;
	extern SmartPtr<dlogstreamstdout> dwarn_ref;
	extern SmartPtr<dlogstreamstderr> derr_ref;
	extern SmartPtr<dlogstreamstdout> ddbg_ref;

	extern SmartPtr<dlogstreamstdout> get_dlog_ref();
#endif

#endif

