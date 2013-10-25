#include "Standard/pch.h"
#include "EmbeddedPtr.h"

EmbeddedPtrHost::EmbeddedPtrHost() :
	_masterRef(new EmbeddedPtrHost*)
{
	*_masterRef = this;
	_self = new EmbeddedPtrBase(this);
}

EmbeddedPtrHost::EmbeddedPtrHost(const EmbeddedPtrHost& copy) :
	_masterRef(new EmbeddedPtrHost*)
{
	*_masterRef = this;
	_self = new EmbeddedPtrBase(this);
}

EmbeddedPtrHost::~EmbeddedPtrHost()
{
	delete _self;
}

