// ------------------------------------------------------------------------------------------------
//
// textstream
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "textstream.h"
#include "Exception/TextStreamParseException.h"
#include "Help.h"
#include "STLHelp.h"
#include <cstring>
#include <sstream>

itextstream::sEndl itextstream::endl;
otextstream::sEndl otextstream::endl;

itextstream::itextstream(const SmartPtr<std::istream>& stream, const std::string& description) :
	_description(description),
	m_stream(stream)
{
	init();
}

itextstream::itextstream(const std::string& string) :
	_description("(string input)"),
	m_stream(new std::istringstream(string))
{
	init();
}

const std::string& itextstream::description() const
{
	return _description;
}

// error
void itextstream::error(const std::string& reason)
{
	throwf(TextStreamParseException(*this, reason));
}

itextstream& itextstream::operator >> (const str_delimiter& d)
{
	bool foundAny = false;
	bool found;

	do
	{
		found = false;

		for (std::string::const_iterator i = d.chars.begin(); i != d.chars.end(); ++i)
		{
			std::string c(1, *i);
			if (has(c))
			{
				foundAny = true;
				found = true;
				*this >> c.c_str();
				break;
			}
		}
	}
	while (found);

	if (foundAny)
		return *this;

	error(std::string("Delimiter character not found (one of '") + join(d.chars.begin(), d.chars.end(), ",") + "').");
	return *this;
}

// token extract
itextstream& itextstream::operator >> (const char* token)
{
	if (!has(token, false))
		error("Token '" + std::string(token) + "' not found");

	return *this;
}

// nextLine
void itextstream::nextLine()
{
//	m_lineNum++;

	int c;
	
	do
	{
		c = s().get();
		
		if (c == '\r')
		{
			if (s().peek() == '\n')
				s().get();
			break;
		}
	}
	while (c != '\n' && s());
}

// has
bool itextstream::has(const std::string& token, bool bPutBack)
{
	return has(token.c_str());
}

bool itextstream::has(const char* token, bool bPutBack)
{
	bool ok = true;

	if (!s())
		return false;

	std::istream::pos_type pos = s().tellg();

	uint size = strlen(token);
	for (uint i = 0; ok && i < size; i++)
	{
		int c;
		
		for(;;)
		{
			c = s().get();
			if (!s())
			{
				ok = false;
				break;
			}

			if (c == token[i])
			{
				break;
			}

			if (m_whitespace.is(c))
			{
				continue;
			}

			if (c != token[i])
			{
				ok = false;
				break;
			}
		}
	}

	if (bPutBack || !ok)
		s().seekg(pos);

	return ok;
}

// skipWhitespace
void itextstream::skipWhitespace(std::string chars)
{
	if (chars.empty())
		chars = m_whitespace.chars;

	int c;
	do
	{
		c = s().get();
	}
	while (s() && chars.find(c) != std::string::npos);

	s().putback(c);
}

// extract to std::string
itextstream& itextstream::operator >> (std::string& obj)
{
	int c;

	obj.resize(0);

	skipWhitespace("\r\n\t");

	bool enclosed = m_enclose_str.m_bEnclose || s().peek() == '"';

	std::string oldDelimiter = m_delimiter.chars;
	std::string oldWhitespace = m_whitespace.chars;
	if (enclosed)
	{
		*this >> "\"";
		*this << str_delimiter("\"");
		*this << whitespace("");
	}

	do
	{
		c = s().get();

		// skip whitespace
		if (m_whitespace.chars.find((char)c) != std::string::npos)
		{
			if (m_delimiter.chars.find((char)c) == std::string::npos)
			{
				continue;
			}
		}

		// end at delimiter
		if (m_delimiter.chars.find((char)c) != std::string::npos)
		{
			s().putback(c);
			break;
		}

		if (s())
		{
			if (enclosed)
			{
				if (c == '\\' && s().peek() == '"')
				{
					s().get();
					c = '"';
				}
			}

			obj.resize(obj.size() + 1);
			obj[obj.size() - 1] = c;
		}
	} while (s());

	if (m_enclose_str.m_bEnclose)
	{
		*this >> m_delimiter;
		*this << str_delimiter(oldDelimiter);
		*this << whitespace(oldWhitespace);
		skipWhitespace();
		skipWhitespace(m_delimiter.chars);
	}

	return *this;
}


void otextstream::init()
{
	m_tabLevel = 0;
}

void otextstream::nextLine()
{ 
	s() << "\n"; 
	
	for (int i = 0; i < m_tabLevel; ++i) 
		s() << "\t"; 
}

int otextstream::tabLevel() const 
{ 
	return m_tabLevel; 
}

void otextstream::tabIn() 
{ 
	++m_tabLevel; 
}

void otextstream::tabOut() 
{ 
	assert(m_tabLevel > 0); 
	--m_tabLevel; 
}
