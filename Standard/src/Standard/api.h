#ifndef STANDARD_API_H
#define STANDARD_API_H

#include <climits>
#include "Standard/platform.h"

#if IS_WINDOWS

	#define WIN32_LEAN_AND_MEAN
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0400
	#endif

	#if IS_MSVC
		#pragma warning(disable:4251)

		// setup dll
		#ifdef STANDARD_STATIC
			#define STANDARD_API
		#else
			#ifndef STANDARD_API
				#define IMPORTING_DLL 1
				#define STANDARD_API __declspec(dllimport)
			#endif
		#endif
	#else
		#define STANDARD_API
	#endif

#elif IS_GNUC

	// setup dll
	#ifdef STANDARD_STATIC
		#define STANDARD_API
	#else
		#ifndef COMPILING_DLL
			#define IMPORTING_DLL 1
		#endif

		//#define STANDARD_API __attribute__((visibility("default")))
		#define STANDARD_API
	#endif
  
#else

	#ifndef STANDARD_STATIC
		#error "Compiler unknown, dynamic library can not be built."
	#endif

#endif

#ifndef NO_RUBY
	extern void STANDARD_API _ruby_backtrace();
#endif

#include <vector>
#include <string>

#endif
