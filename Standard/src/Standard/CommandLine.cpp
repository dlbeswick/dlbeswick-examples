#include "Standard/pch.h"
#include "CommandLine.h"
#include "Standard/Exception.h"

void CommandLine::init()
{
	_nullCStr = "";
	_null = _nullCStr;
	_argv = 0;
}

CommandLine::CommandLine()
{
	init();
}

CommandLine::CommandLine(const char* lpCmdLine)
{
	init();
	throwf("Not implemented");
}

CommandLine::CommandLine(int argc, const char** argv)
{
	init();
	_args.insert(_args.begin(), argv, argv + argc);
}

CommandLine::~CommandLine()
{
	delete[] _argv;
}

void CommandLine::append(const std::string& arg)
{
	_args.push_back(arg);
}

uint CommandLine::argc() const
{
	return _args.size();
}

const char* const* CommandLine::argv()
{
	if (_argv)
		delete[] _argv;

	if (_args.empty())
	{
		return &_nullCStr;
	}
	else
	{
		_argv = new char*[_args.size() + 1];

		uint i = 0;

		for (; i < _args.size(); ++i)
			_argv[i] = (char*)_args[i].c_str();

		_argv[i] = 0;

		return _argv;
	}
}

bool CommandLine::empty() const
{
	return _args.empty();
}

bool CommandLine::has(const std::string& arg)
{
	return std::find(_args.begin(), _args.end(), arg) != _args.end();
}

void CommandLine::insert(int idx, const std::string& arg)
{
	_args.insert(_args.begin() + idx, arg);
}

void CommandLine::prepend(const std::string& arg)
{
	_args.insert(_args.begin(), arg);
}

void CommandLine::removeArg(const std::string& arg)
{
	do
	{
		std::vector<std::string>::iterator it = std::find(_args.begin(), _args.end(), arg);

		if (it != _args.end())
		{
			_args.erase(it);
		}
		else
		{
			break;
		}
	}
	while (true);
}

uint CommandLine::size() const
{
	return _args.size();
}

const std::string& CommandLine::operator[] (uint idx) const
{
	if (idx < 0 || idx >= _args.size())
		return _null;
	else
		return _args[idx];
}

std::string& CommandLine::operator[] (uint idx)
{
	if (idx < 0 || idx >= _args.size())
		return _null;
	else
		return _args[idx];
}

void CommandLine::insertSlice(int destIdx, const CommandLine& src, int srcStart, int srcEnd)
{
	for (int i = srcStart; i != srcEnd; ++i, ++destIdx)
	{
		insert(destIdx, src[i]);
	}
}
