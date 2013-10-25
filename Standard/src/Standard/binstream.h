// ------------------------------------------------------------------------------------------------
//
// binstream
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/api.h"
#include <ostream>
#include <istream>
#include "Standard/SmartPtr.h"


class STANDARD_API ibinstream
{
public:
	ibinstream(const SmartPtr<std::istream>& stream) :
		m_stream(stream)
	{
	}

	std::istream& s() { return *m_stream; }
	const std::istream& s() const { return *m_stream; }

	// allocates a buffer and dumps stream contents
	void toBuf(char*& buf, size_t& length);

private:
	SmartPtr<std::istream> m_stream;
};

class STANDARD_API obinstream
{
public:
	obinstream(const SmartPtr<std::ostream>& stream) :
		m_stream(stream)
	{
	}

	std::ostream& s() { return *m_stream; }
	const std::ostream& s() const { return *m_stream; }

private:
	SmartPtr<std::ostream> m_stream;
};
