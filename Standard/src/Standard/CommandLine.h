#ifndef _STANDARD_COMMANDLINE_H
#define _STANDARD_COMMANDLINE_H

#include "Standard/api.h"
#include "Standard/Help.h"

class STANDARD_API CommandLine
{
public:
	CommandLine();
	CommandLine(const char* lpCmdLine);
	CommandLine(int argc, const char** argv);
	~CommandLine();

	void append(const std::string& arg);

	uint argc() const;

	const char* const* argv();

	bool empty() const;

	bool has(const std::string& arg);

	void insert(int idx, const std::string& arg);

	void prepend(const std::string& arg);

	void removeArg(const std::string& arg);

	uint size() const;

	void insertSlice(int destIdx, const CommandLine& src, int srcStart, int srcEnd);

	const std::string& operator[] (uint idx) const;

	std::string& operator[] (uint idx);

protected:
	void init();

	char** _argv;
	const char* _nullCStr;
	std::string _null;
	std::vector<std::string> _args;
};

#endif
