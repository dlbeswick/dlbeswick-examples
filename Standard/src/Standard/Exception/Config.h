#ifndef STANDARD_EXCEPTION_CONFIG_H
#define STANDARD_EXCEPTION_CONFIG_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"

class Config;

class STANDARD_API ExceptionConfig : public ExceptionStream
{
public:
	ExceptionConfig(const Config& what);
};

class STANDARD_API ExceptionConfigSave : public ExceptionConfig
{
public:
	ExceptionConfigSave(const Config& what);
};

#endif
