#include "Standard/pch.h"
#include "Log.h"
#include "Path.h"
#include "STLHelp.h"
#include <time.h>

#include <iostream>
#include <fstream>

STANDARD_API SmartPtr<dlogstreamstdout> dlog_ref = get_dlog_ref();
STANDARD_API SmartPtr<dlogstreamstdout> dwarn_ref = new dlogstreamstdout;
STANDARD_API SmartPtr<dlogstreamstderr> derr_ref = new dlogstreamstderr;
STANDARD_API SmartPtr<dlogstreamstdout> ddbg_ref = new dlogstreamstdout;

STANDARD_API dlogstreamstdout& dlog = *dlog_ref;
STANDARD_API dlogstreamstdout& dwarn = *dwarn_ref;
STANDARD_API dlogstreamstderr& derr = *derr_ref;
STANDARD_API dlogstreamstdout& ddbg = *ddbg_ref;

SmartPtr<dlogstreamstdout> get_dlog_ref()
{
	if (!dlog_ref)
		dlog_ref = new dlogstreamstdout;

	return dlog_ref;
}

// dlogprinterfile
dlogprinterfile::dlogprinterfile(const Path& fname) :
	s(new std::ofstream((*fname).c_str()))
{
}

dlogprinterfile::dlogprinterfile(std::ostream* _s) :
	s(_s)
{
}

dlogprinterfile::~dlogprinterfile()
{
	delete s;
}

void dlogprinterfile::operator()(const char* data, int len)
{
	s->write(data, len);
	s->flush();
}

// dlogprinterdebug
void dlogprinterdebug::operator()(const char* data, int len)
{
#if IS_WINDOWS
	OutputDebugString(std::string(data, len).c_str());
#endif
}

// stdout
void dlogprinterstdout::operator()(const char* data, int len)
{
	std::cout.write(data, len);
	std::cout.flush();
}

// stderr
void dlogprinterstderr::operator()(const char* data, int len)
{
	std::cerr.write(data, len);
	std::cerr.flush();
}

dlogstream::internalstreambuf::internalstreambuf(std::ios_base::openmode which) :
	m_stream(0),
	std::stringbuf(which)
{
}

dlogstream::internalstreambuf::internalstreambuf(const std::string& str, std::ios_base::openmode which) :
	m_stream(0),
	std::stringbuf(str, which)
{
}

// dlogstream::internalstreambuf
dlogstream::internalstreambuf::~internalstreambuf()
{
	freeSTL(m_printers);
}

void dlogstream::internalstreambuf::add(dlogprinter* p)
{
	m_printers.push_back(p);
}

// dlogstream
dlogstream::dlogstream(bool useTimestamp) : 
	otextstream(new internalstream),
	m_enabled(true),
	m_useTimestamp(useTimestamp)
{
}

dlogstream::~dlogstream()
{
	s().flush();
}

bool dlogstream::shouldPrint()
{
	return m_enabled;
}

int dlogstream::internalstreambuf::sync()
{
	if (!m_stream)
		return 0;

	if (!m_stream->shouldPrint())
		return 0;

	if ((int)m_stream->s().tellp() == -1)
		return 0;

	std::stringbuf::sync();

	if (m_stream->m_useTimestamp)
	{
		time_t curtime;
		struct tm *loctime;

		curtime = time (NULL);
		loctime = localtime (&curtime);

		std::string printStr = asctime(loctime);

		for (uint i = 0; i < m_printers.size(); ++i)
		{
			(*m_printers[i])(printStr.c_str(), printStr.size());
		}
	}
	
	for (uint i = 0; i < m_printers.size(); ++i)
	{
		(*m_printers[i])(str().c_str(), (int)m_stream->s().tellp());
	}

	seekpos(0);
	return 0;
}

void dlogstream::add(dlogprinter* p)
{
	internalstreambuf& buf = (internalstreambuf&)*s().rdbuf();
	buf.add(p);
	buf.m_stream = this;
}

void dlogstream::setEnabled(bool b)
{
	m_enabled = b;
}

void dlogstream::redirect(std::ostream& source)
{
	source.rdbuf(s().rdbuf());
}

#if IS_WINDOWS
void dlogstream::attachToConsole()
{
	add(new dlogprinterfile(new std::ofstream("CONOUT$")));
}
#endif

////

dlogstreamstdout::dlogstreamstdout()
{
	// use debug printer and stdout as default
	add(new dlogprinterdebug);
	add(new dlogprinterstdout);
}

////

dlogstreamstderr::dlogstreamstderr()
{
	// use debug printer and stderr as default
	add(new dlogprinterdebug);
	add(new dlogprinterstderr);
}

////
