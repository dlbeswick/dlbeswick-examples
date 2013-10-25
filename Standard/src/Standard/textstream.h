// ------------------------------------------------------------------------------------------------
//
// textstream classes
// class wrappers to differentiate between text and binary formatted files
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include <string>
#include <assert.h>
#include <sstream>
#include "Standard/SmartPtr.h"

// itextstream
class STANDARD_API itextstream
{
public:
	itextstream(const SmartPtr<std::istream>& stream, const std::string& description = "(unknown)");

	itextstream(const std::string& string);

	std::istream& s() { return *m_stream; }
	const std::istream& s() const { return *m_stream; }

	static struct sEndl {} endl;

	// returns the value of the stream's 'good'
	operator void* () { return s(); }

	// test for a token, put chars back
	bool has(const std::string& token, bool bPutBack = true);
	bool has(const char* token, bool bPutBack = true);

	const std::string& description() const;

	// go to the next line in the file
	void nextLine();

	// skip the whitespace
	void skipWhitespace(std::string chars = std::string());

	// fail if token not found
	itextstream& operator >> (const char* token);
	
	// don't use char buffers, use a std::string instead
	itextstream& operator >> (char* buf)
	{
		assert(0);
		return *this;
	}

	itextstream& operator >> (sEndl&)
	{
		nextLine();
		return *this;
	}

	// stream modes
	// selects the delimiters for string reads
	struct str_delimiter
	{
		str_delimiter(const std::string& s = " \n") :
			chars(s)
		{
		}

		std::string chars;
	} m_delimiter;

	/// Read any one of the characters in the delimiter object.
	itextstream& operator >> (const str_delimiter& d);

	itextstream& operator << (const str_delimiter& m)
	{
		m_delimiter = m;
		return *this;
	}

	// defines the characters that are considered to be whitespace
	// whitespace is ignored during token and string reads
	// default is that all whitespace matters
	struct whitespace
	{
		whitespace(const std::string& s = " \n\t") :
			chars(s)
		{
		}

		bool is(char c) { return chars.find(c) != std::string::npos; }

		std::string chars;
	} m_whitespace;

	itextstream& operator << (const whitespace& m)
	{
		m_whitespace = m;
		return *this;
	}

	// enclose strings with quotes
	struct enclose_str
	{
		enclose_str(bool bEnclose = false) : m_bEnclose(bEnclose) {}

		bool m_bEnclose;
	} m_enclose_str;

	itextstream& operator << (const enclose_str& m)
	{
		m_enclose_str = m;
		return *this;
	}

	// string read with delimiter and whitespace ignored
	itextstream& operator >> (std::string& obj);

protected:
	void init() {};
	void error(const std::string& reason);

	SmartPtr<std::istream> m_stream;
	std::string _description;
};


// otextstream
class STANDARD_API otextstream
{
public:
	otextstream(const SmartPtr<std::ostream>& stream) :
		m_stream(stream)
	{
		init();
	}

	otextstream(const std::string& string) :
		m_stream(new std::ostringstream(string))
	{
		init();
	}

	std::ostream& s() { return *m_stream; }
	const std::ostream& s() const { return *m_stream; }

	static struct sEndl {} endl;

	// tab level
	void nextLine();
	int tabLevel() const;
	void tabIn();
	void tabOut();

	// end line
	otextstream& operator << (sEndl&)
	{
		s() << std::endl;
		return *this;
	}

	// enclose strings with quotes
	struct enclose_str
	{
		enclose_str(bool bEnclose = false) : m_bEnclose(bEnclose) {}

		bool m_bEnclose;
	} m_enclose_str;

	otextstream& operator << (const enclose_str& m)
	{
		m_enclose_str = m;
		return *this;
	}

private:
	void init();

	int m_tabLevel;
	SmartPtr<std::ostream> m_stream;
};
