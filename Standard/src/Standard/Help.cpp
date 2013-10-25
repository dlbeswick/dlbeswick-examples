// ------------------------------------------------------------------------------------------------
//
// Help
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Help.h"

// from amd optimisation and profiling example
STANDARD_API std::string getCPUVendorID()
{
#if IS_X86 && IS_MSVC
	char	szVendor[64];

	__asm
	{
		lea		esi,szVendor

		mov		eax,0x80000002		// FIRST 16 CHRS
		xor		ebx,ebx
		xor		ecx,ecx
		xor		edx,edx

		_emit	0x0f
		_emit	0xa2

		mov		[esi],eax
		mov		[esi+4],ebx
		mov		[esi+8],ecx
		mov		[esi+12],edx

		add		esi,16
		mov		eax,0x80000003		// NEXT 16 CHRS
		xor		ebx,ebx
		xor		ecx,ecx
		xor		edx,edx

		_emit	0x0f
		_emit	0xa2

		mov		[esi],eax
		mov		[esi+4],ebx
		mov		[esi+8],ecx
		mov		[esi+12],edx


		add		esi,16
		mov		eax,0x80000004		// LAST 16 CHRS
		xor		ebx,ebx
		xor		ecx,ecx
		xor		edx,edx

		_emit	0x0f
		_emit	0xa2

		mov		[esi],eax
		mov		[esi+4],ebx
		mov		[esi+8],ecx
		mov		[esi+12],edx

		lea		esi,szVendor
		mov		eax,0
		mov		[esi+63],al
	}

	return szVendor;
#else
	char szVendor[64];

#if !IS_X64
	asm
	(
		"mov		%0,%%esi;"

		"mov		$0x80000002,%%eax;"		// FIRST 16 CHRS
		"xor		%%ebx,%%ebx;"
		"xor		%%ecx,%%ecx;"
		"xor		%%edx,%%edx;"

		"cpuid;"

		"mov		%%eax,(%%esi);"
		"mov		%%ebx,4(%%esi);"
		"mov		%%ecx,8(%%esi);"
		"mov		%%edx,12(%%esi);"

		"add		$16,%%esi;"
		"mov		$0x80000003,%%eax;"		// NEXT 16 CHRS
		"xor		%%ebx,%%ebx;"
		"xor		%%ecx,%%ecx;"
		"xor		%%edx,%%edx;"

		"cpuid;"

		"mov		%%eax,(%%esi);"
		"mov		%%ebx,4(%%esi);"
		"mov		%%ecx,8(%%esi);"
		"mov		%%edx,12(%%esi);"

		"add		$16,%%esi;"
		"mov		$0x80000004,%%eax;"		// LAST 16 CHRS
		"xor		%%ebx,%%ebx;"
		"xor		%%ecx,%%ecx;"
		"xor		%%edx,%%edx;"

		"cpuid;"

		"mov		%%eax,(%%esi);"
		"mov		%%ebx,4(%%esi);"
		"mov		%%ecx,8(%%esi);"
		"mov		%%edx,12(%%esi);"

		"mov		$0,%%eax;"
		"movb		%%al,16(%%esi);"
		:
		:"r"(szVendor)
		:
	);
#else
	strcpy(szVendor, "64-bit support pending.");
#endif

	return szVendor;
#endif
}

static int getTime()
{
#if IS_WINDOWS
	return timeGetTime();
#else
	portRequired("getTime");
	return 0;
#endif
}

STANDARD_API unsigned long getCPUTicksPerSecond()
{
	int	timeStart	= 0;
	int	timeStop	= 0;
	dword StartTicks	= 0;
	dword EndTicks	= 0;
	dword TotalTicks	= 0;

	timeStart = getTime();				//GET TICK EDGE
	for(;;)
	{
		timeStop = getTime();
		if ( (timeStop-timeStart) > 1 )		// ROLLOVER PAST 1
		{
		#if IS_X86 && IS_MSVC
			__asm{
					xor    eax, eax
					xor    ebx, ebx
					xor    ecx, ecx
					xor    edx, edx
					_emit  0x0f				// CPUID
					_emit  0xa2
					_emit  0x0f				// RDTSC
					_emit  0x31
					mov    [StartTicks], eax
				}
			break;
		#elif IS_X86 && IS_GNUC
			asm (
					"xor    %%eax,%%eax\n\t"
					"xor    %%ebx,%%ebx\n\t"
					"xor    %%ecx,%%ecx\n\t"
					"xor    %%edx,%%edx\n\t"
					"cpuid\n\t"				// CPUID
					"rdtsc\n\t"				// RDTSC
					"mov   %%eax, %0\n\t"
					:"=r"(StartTicks)
					:
					:
			);
			break;
		#else
			portRequired();
		#endif
		}
	}

	timeStart = timeStop;

	for(;;)
	{
		timeStop = getTime();
		if ( (timeStop-timeStart) > 1000 )	//ONE SECOND
		{
		#if IS_X86 && IS_MSVC
			__asm{
					xor    eax, eax
					xor    ebx, ebx
					xor    ecx, ecx
					xor    edx, edx
					_emit  0x0f				// CPUID
					_emit  0xa2
					_emit  0x0f				// RDTSC
					_emit  0x31
					mov    [EndTicks], eax
				}
#elif IS_X86 && IS_GNUC
			asm (
					"xor    %%eax,%%eax\n\t"
					"xor    %%ebx,%%ebx\n\t"
					"xor    %%ecx,%%ecx\n\t"
					"xor    %%edx,%%edx\n\t"
					"cpuid\n\t"				// CPUID
					"rdtsc\n\t"				// RDTSC
					"mov   %%eax, %0\n\t"
					:"=r"(StartTicks)
					:
					:
			);
			break;
		#else
			portRequired();
		#endif

			break;
		}
	}

	TotalTicks = EndTicks-StartTicks;		// TOTAL

	return TotalTicks;
}
