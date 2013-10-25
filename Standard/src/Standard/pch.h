#define COMPILING_DLL
#include "Standard/api.h"
#include "Standard/Help.h"

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <limits>
#include <new>
#include <string>
#include <deque>
#include <stack>
#include <vector>
#include <map>
#include <set>
#include <utility>

#if IS_WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#include <windows.h>
	#include <shellapi.h>
	#include <mmsystem.h>
	#include <d3d9types.h>

	#if HAS_DBGHELP
		#include <dbghelp.h>
	#endif
#endif

using std::abs;

#if IS_MSVC
#pragma warning(disable:4251)
#pragma warning(disable:4503)
#endif

#if IS_OSX
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreServices/CoreServices.h>
#endif
