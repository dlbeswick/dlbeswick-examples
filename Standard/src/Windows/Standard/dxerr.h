#include "Standard/api.h"

#if IS_GNUC
extern "C"
{
	STANDARD_API const TCHAR *DXGetErrorString9(unsigned long hr);
	STANDARD_API const TCHAR *DXGetErrorDescription9(unsigned long hr);
}
#else
	#include <dxerr9.h>
#endif
