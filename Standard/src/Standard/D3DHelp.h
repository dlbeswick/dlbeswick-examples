// ---------------------------------------------------------------------------------------------------------
//
// D3DHelp
//
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_D3DHELP_H
#define STANDARD_D3DHELP_H


#include "Standard/api.h"
#if !IS_GNUC // mingw/wine whinges about redefinitions: fix this
	#include <d3d.h>
#endif

inline DWORD D3DCOL(float r, float g, float b, float a = 1.0f) { return D3DCOLOR_COLORVALUE(r,g,b,a); }
// maybe #ifdef _DEBUG this out for final release?
#define DXFAIL(x) x < 0 && MessageBox(0, #x, "DirectX Call Failed", 0) && MessageBox(0, __FILE__, 0, 0)

#endif
