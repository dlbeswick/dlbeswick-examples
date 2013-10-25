#if DSOUND_SWIG_CAPABILITIES

// tbd: fill this in by configuration step
#define WITH_SOX 1
#define WITH_FLAC 1

#else

#include <Standard/platform.h>

//#define WITH_SOX 1

#if IS_WINDOWS
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0400
	#endif

	#ifdef NOMP3
		#define HAVE_BLADEENC 0
	#else
		#define HAVE_BLADEENC 1
	#endif
#endif

#endif

