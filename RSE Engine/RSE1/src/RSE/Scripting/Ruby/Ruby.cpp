// ------------------------------------------------------------------------------------------------
//
// Ruby
//
// ------------------------------------------------------------------------------------------------
#if IS_MSVC
#include "stdafx.h"
#include <Standard/Log.h>

#include "Ruby.h"
//#include "Ruby/1.8/i386-mswin32/ruby.h"

Ruby::Ruby()
{
#if defined(NT)
/*	int argc = 1;
	LPTSTR cmdLine = GetCommandLine();
	LPTSTR* pCmdLine = &cmdLine;
	NtInitialize(&argc, &pCmdLine);*/
#endif

//	ruby_init();
//	dlog << "Initialized Ruby interpreter.\n";
}

Ruby::~Ruby()
{
}
#endif
