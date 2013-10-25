#ifndef STANDARD_PLATFORM_H
#define STANDARD_PLATFORM_H

#include <cstdint>

#define IS_GNUC 		__GNUC__
#define IS_MSVC 		_MSC_VER
#define IS_WINDOWS 		(_WIN32 || WIN32)
#define IS_X86 			!__ppc__
#define IS_OSX 			_APPLE_
#define IS_LINUX 		__linux__
#define IS_CXX0X 		!(__cplusplus == 199711L)
#define IS_POSIX 		!IS_WINDOWS
#define IS_X64 			((IS_GNUC && __LP64__) || (IS_WINDOWS /*fix this, add 64-bit windows*/))
#define IS_X86_32 		IS_X86 && !IS_X64

#define STANDARD_PLATFORM_INFO 0
#if STANDARD_PLATFORM_INFO
	#if IS_MSVC
		#pragma message "Platform: IS_MSVC"
	#endif

	#if IS_GNUC
		#pragma message "Platform: IS_GNUC"
	#endif

	#if IS_WINDOWS
		#pragma message "Platform: IS_WINDOWS"
	#endif

	#if IS_X86
		#pragma message "Platform: IS_X86"
	#endif

	#if IS_OSX
		#pragma message "Platform: IS_OSX"
	#endif

	#if IS_LINUX
		#pragma message "Platform: IS_LINUX"
	#endif

	#if IS_CXX0X
		#pragma message "Platform: IS_CXX0X"
	#endif

	#if IS_POSIX
		#pragma message "Platform: IS_POSIX"
	#endif
#endif

// hash map platform differences
#if IS_GNUC
	#undef read
	#undef write

	#if IS_CXX0X
		#include <unordered_map>
		#include <unordered_set>

		namespace stdext
		{
			template <class T, class U> class hash_map : public std::unordered_map<T, U> {};
			template <class T, class U> class hash_multimap : public std::unordered_multimap<T, U> {};
			template <class T> class hash_set : public std::unordered_set<T> {};
		}
	#else
		#include <ext/hash_map>
		#include <ext/hash_set>
		namespace __gnu_cxx
		{
			template<>
				struct hash<std::string>
				{
					size_t
					operator() (const std::string& __s) const
					{ return __stl_hash_string(__s.c_str()); }
				};

			template<class T>
				struct hash<const T*>
				{
					size_t
					operator() (const T* const& __x) const
					{ return (unsigned long)__x; }
				};

		};

		namespace stdext = __gnu_cxx;
	#endif

	#ifndef FLT_MIN
		#define FLT_MIN std::numeric_limits<float>::min()
		#define FLT_MAX std::numeric_limits<float>::max()
	#endif

	#define __forceinline inline

	#define FUNCTION_EXPORT __attribute__((visibility("default")))

#else
	// disables 'resolved overload was found by argument-dependent lookup', caused by use of visual studio hash_map
	#pragma warning(disable:4675)

	#include <hash_map>
	#include <hash_set>

	#define PATH_MAX _MAX_PATH
	#define FUNCTION_EXPORT

	typedef __int64 int64;
	//typedef unsigned long long uint64; not available?
#endif

#if IS_OSX
	#include <unistd.h>
	#define Sleep usleep
	#define STANDARD_RUBY_NO_RELATIVE_LIB_DEFS 1
#endif

#if IS_WINDOWS
	#define HAS_DBGHELP !IS_GNUC
#endif

#include <stdexcept>

inline void portRequired(const std::string& info)
{
	throw(std::runtime_error(std::string("Porting is required. (") + info + ")"));
}

#if !NO_BASIC_TYPEDEFS
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int dword;
#endif

#if IS_MSVC
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else
	typedef int64_t int64;
	typedef uint64_t uint64;
#endif

#endif
