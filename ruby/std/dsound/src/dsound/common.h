#ifndef DSOUND_COMMON_H
#define DSOUND_COMMON_H

#undef write
#undef read
#undef close

#include "dsound/platform_common.h"
#include "dsound/capabilities.h"
#include "Standard/api.h"
#include "Standard/Log.h"

#include <cassert>
#include <cstring>
#include <cmath>
using std::abs;

#if WITH_SOX
#include <sox.h>
#endif

#if IS_LINUX
#include <portaudio.h>
#include <unistd.h>
#endif

#define ensureDS(x) { int result = x; if (result != DS_OK) throw(ExceptionString(std::string(#x) + std::string(" ") + std::string(DXGetErrorDescription9(result)))); }

///

extern "C"
{
	#include "id3tag.h"
	#undef SIZEOF_LONG_LONG
	#include "mad.h"
}

#include <map>
#include <functional>
#include <fstream>
#include <vector>
#include <set>

// it's a pain until the code is updated for c++0x. backward compatibility is in Standard.
//#if IS_GNUC
//	#include <ext/hash_map>
//#else
//	#include <hash_map>
//#endif

#include <numeric>
#include <assert.h>
#include <limits.h>

#include <fstream>
#include "Standard/AsyncEvent.h"
#include "Standard/CircularBuffer.h"
#include "Standard/CriticalSection.h"
#include "Standard/CriticalSectionBlock.h"
#include "Standard/Exception.h"
#include "Standard/Help.h"
#include <Standard/Interpolation.h>
#include "Standard/Lockable.h"
#include <Standard/Mapping.h>
#include <Standard/Math.h>
#include <Standard/MathHelp.h>
#include "Standard/PtrGC.h"
#include "Standard/STLHelp.h"
#include "Standard/Thread.h"

#if IS_WINDOWS
	extern HINSTANCE g_dllInstance;
#endif

#endif
