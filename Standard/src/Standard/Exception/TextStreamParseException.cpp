#include "Standard/pch.h"
#include "TextStreamParseException.h"
#include "Standard/textstream.h"
#include "Standard/Log.h"
#include <sstream>

TextStreamParseException::TextStreamParseException(itextstream& stream, const std::string& reason) :
	ExceptionString(reason),
	_stream(stream)
{
}

const char* TextStreamParseException::what() const throw()
{
	if (_finalMessage.empty())
	{
		std::stringstream lineNumInfo;
		std::string lastLine;

		if (!_stream.s().good())
		{
			lastLine = "(eof, no info available)";
			lineNumInfo << "(unknown)";
		}
		else
		{
			std::ios::pos_type errorPos = _stream.s().tellg();
			std::ios::pos_type lastLinePos = errorPos;

			// find last linebreak
			while (_stream.s().peek() != (int)'\n' && _stream.s().peek() != (int)'\r' && (int)_stream.s().tellg() != 0 && (errorPos - lastLinePos < 75))
			{
				_stream.s().seekg(-1, std::ios::cur);
				lastLinePos -= 1;
			}

			_stream.s().seekg(1, std::ios::cur);
			lastLinePos += 1;

			// get bad line
			char line[80];
			_stream.s().read(line, 80);

			lastLine.append(line);

			// show error position if possible
			size_t strErrorPos = (size_t)(errorPos - lastLinePos);
			if (strErrorPos < lastLine.size())
				lastLine = lastLine.substr(0, strErrorPos) + "**" + lastLine[strErrorPos] + "**" + lastLine.substr(strErrorPos + 1);

			// write file to error, for extra debugging
			_stream.s().clear();
			_stream.s().seekg(0);

			derr.s() << "Parse error, file follows:\n" << _stream.s().rdbuf() << std::endl;

			// count linebreaks to get line number
			int lineNum = 1;

			_stream.s().clear();
			_stream.s().seekg(0);

			while (_stream.s().good() && _stream.s().tellg() < errorPos)
			{
				char c[3];
				
				_stream.s().get(c, 2);
				
				if (c[0] == '\r')
				{
					++lineNum;
					
					if (_stream.s().peek() != -1 && strlen(c) > 1)
						_stream.s().seekg(-1, std::ios::cur);
				}
				else if (c[0] == '\r' && c[1] == '\n')
				{
					++lineNum;
				}
			}

			lineNumInfo << lineNum;
		}

		_finalMessage = std::string("Parse error: ") + message + ", desc: " + _stream.description() + ", line: " + lineNumInfo.str() + ", detail: " + lastLine;

		derr.s() << "Parse error description: " << _finalMessage << std::endl;
	}
	
	return _finalMessage.c_str();
}
