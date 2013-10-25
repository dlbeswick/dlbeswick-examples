#include "Standard/pch.h"
#include "Standard/Exception/Config.h"
#include "Standard/Config.h"

ExceptionConfig::ExceptionConfig(const Config& what)
{
	addContext(what.description());
}

ExceptionConfigSave::ExceptionConfigSave(const Config& what) :
	ExceptionConfig(what)
{
}
