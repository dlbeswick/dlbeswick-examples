#include "pch.h"
#include "PathResource.h"
#include "AppBase.h"
#include <Standard/FileMgr.h>

PathResource::PathResource(const std::string& stringRep) :
	Path(stringRep, AppBase().filesystem())
{
}
