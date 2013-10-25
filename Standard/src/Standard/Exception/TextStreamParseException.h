#ifndef STANDARD_TEXTSTREAMPARSEEXCEPTION_H
#define STANDARD_TEXTSTREAMPARSEEXCEPTION_H

#include "Standard/api.h"
#include "Standard/Exception.h"

class itextstream;

class TextStreamParseException : public ExceptionString
{
public:
	TextStreamParseException(itextstream& stream, const std::string& reason);
	virtual ~TextStreamParseException() throw() {}

	virtual const char* what() const throw();

protected:
	itextstream& _stream;
	mutable std::string _finalMessage;
};

#endif
