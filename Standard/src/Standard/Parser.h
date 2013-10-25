// ------------------------------------------------------------------------------------------------
//
// Parser
// Text file format parser
// The small amount of functionality in this class means it should probably be incorporated into itextstream.
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "SmartPtr.h"
#include <string>
#include <assert.h>
#include "textstream.h"


class STANDARD_API Parser
{
public:
	Parser(itextstream& stream);

	int				lineNum()	{ return m_lineNum; }
	itextstream&	stream()	{ return m_stream; }

private:
	void error(const std::string& reason);

	std::string		m_input;
	itextstream&	m_stream;
	int				m_lineNum;
};