#ifndef RSE_EXCEPTION_VIDEO_H
#define RSE_EXCEPTION_VIDEO_H

#include <Standard/ExceptionStream.h>

// tbd: fix ExceptionStream implementation to allow more than one level of derivation
class RSE_API ExceptionVideo : public ExceptionStream
{
};

class RSE_API ExceptionD3D : public ExceptionStream
{
public:
	ExceptionD3D(int _resultCode);

	const std::string context() const;

	int resultCode;
};

#define DX_ENSURE(operation) \
{ \
	int dxensure_result = operation;\
	if (dxensure_result != D3D_OK) \
		EXCEPTIONSTREAM(ExceptionD3D(dxensure_result), #operation); \
} \

#endif