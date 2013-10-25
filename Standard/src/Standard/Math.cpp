// ------------------------------------------------------------------------------------------------
//
// Math
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Math.h"
using namespace DMath;

float idm2[4] = {1,0,0,1};
template<> const TMatrix2<float> TMatrix2<float>::IDENTITY(idm2);

float idm3[9] = {1,0,0,0,1,0,0,0,1};
template<> const TMatrix3<float> TMatrix3<float>::IDENTITY(idm3);

float idm4[16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
template<> const TMatrix4<float> TMatrix4<float>::IDENTITY(idm4);

template<> const TPoint4<float> TPoint4<float>::ZERO(0, 0, 0, 0);
template<> const TPoint4<float> TPoint4<float>::ONE(1, 1, 1, 1);
template<> const TPoint4<float> TPoint4<float>::MAX(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
template<> const TPoint3<float> TPoint3<float>::ZERO(0, 0, 0);
template<> const TPoint3<float> TPoint3<float>::ONE(1, 1, 1);
template<> const TPoint3<float> TPoint3<float>::MAX(FLT_MAX, FLT_MAX, FLT_MAX);
template<> const TPoint2<float> TPoint2<float>::ZERO(0, 0);
template<> const TPoint2<float> TPoint2<float>::ONE(1, 1);
template<> const TPoint2<float> TPoint2<float>::MAX(FLT_MAX, FLT_MAX);
template<> const TPoint2<int> TPoint2<int>::ZERO(0, 0);
template<> const TPoint2<int> TPoint2<int>::ONE(1, 1);
template<> const TPoint2<int> TPoint2<int>::MAX(INT_MAX, INT_MAX);

template<> const TQuat<float> TQuat<float>::IDENTITY(1, 0, 0, 0);
template<> const TQuat<float> TQuat<float>::NONE(0, 0, 0, 0);
