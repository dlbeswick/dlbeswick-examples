#include "pch.h"
#include "Video.h"
#include "Standard/dxerr.h"

ExceptionD3D::ExceptionD3D(int _resultCode)
{
	resultCode = _resultCode;

	addContext(DXGetErrorString9(resultCode));
	addContext(DXGetErrorDescription9(resultCode));
}
