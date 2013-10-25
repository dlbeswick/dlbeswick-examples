// ------------------------------------------------------------------------------------------------
//
// ADVANCED REALITY SIMULATION ENGINE
//
// ------------------------------------------------------------------------------------------------
#ifndef RSE_RSE_H
#define RSE_RSE_H

#if IS_MSVC
#pragma warning(disable:4503)
#pragma warning(disable:4786)
#pragma warning(disable:4251)
#pragma warning(disable:4250) // multiple inheritance: dominance warning (usually benign)
#pragma warning(disable:4675)
#pragma warning(3:4714)
#pragma inline_recursion(on)
#pragma inline_depth(255)
#endif

#define WIN32_LEAN_AND_MEAN

// setup dll
#ifndef RSE_API
	#define IMPORTING_DLL 1
	#if IS_MSVC
		#define RSE_API __declspec(dllimport)
	#else
		#define RSE_API
	#endif
	#define RSE_API_SHARED 
#else
	#define RSE_API_SHARED RSE_API
	#define COMPILING_DLL 1
#endif

// Project options
#define USE_FIBERS 1

// Windows headers
#if USE_FIBERS
	#define _WIN32_WINNT 0x0400 
#endif

// Third-party APIs
#include <fmod/fmodapi374win/api/inc/fmod.h>

#include "Standard/platform.h"
// tbd: remove/replace this
//#include "Standard/greta/regexpr2.h"

#include <windows.h>
#include <winsock2.h>
#include <psapi.h>
#include <assert.h>
#if !IS_MINGW_WINE
#include <crtdbg.h>
#endif
#include <winuser.h>

#if _DEBUG // enable dx9 debug info
#define D3D_DEBUG_INFO
#endif

#if IS_GNUC
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif
#include <d3d9.h>
#include <d3dx9.h>
#if IS_GNUC
#pragma GCC diagnostic warning "-Wunknown-pragmas"
#endif

// Standard library
#include <memory>
#include <deque>
#include <vector>
#include <string>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <ios>
#include <stack>
#include <sstream>

#include <boost/regex.hpp>

// Externs
extern HINSTANCE g_hRseDLL;

#endif
