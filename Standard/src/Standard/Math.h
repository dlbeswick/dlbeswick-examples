#ifndef STANDARD_MATH_H
#define STANDARD_MATH_H

#undef minor

#include "Standard/api.h"
#include "Help.h"
#include <cmath>
#include <cassert>

using std::abs;

static const float EPSILON = 0.000001f;

/*#pragma inline_recursion(on)
#pragma inline_depth(255)*/

namespace DMath
{
#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

//-----------------------------------------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------------------------------------

const float PI = 3.1415926535897932384626433832795f;

template <class T>
T fimod(T val, int divisor) { return val - divisor * ((int)val / divisor); }

template <class T>
T degToRad(T val) { return PI * val / (T)180.0; }

template <class T>
T radToDeg(T val) { return (T)180.0 * val / PI; }

template <class T>
T sign(T val) { if (val >= 0) return 1; else return -1; }

//-----------------------------------------------------------------------------------------------------------
// TPoint4
//-----------------------------------------------------------------------------------------------------------

template <class T> class TPoint2;
template <class T> class TPoint3;

template <class T>
class TPoint4
{
public:
	typedef T Element;

	T x, y, z, w;

	TPoint4() {}
	TPoint4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
	TPoint4(const TPoint4 &p) : x(p.x), y(p.y), z(p.z), w(p.w) {}

	STANDARD_API static const TPoint4 ZERO;
	STANDARD_API static const TPoint4 ONE;
	STANDARD_API static const TPoint4 MAX;

	operator T *()							{ return &x; }

	TPoint4& operator = (const TPoint4 &p)	{ x = p.x; y = p.y; z = p.z; w = p.w; return *this; }

	// index addressing
	T& operator[] (int idx)					{ return *(&x + idx); }
	const T& operator[] (int idx) const 	{ return *(&x + idx); }

	bool operator == (const TPoint4 &p)	const	{ return x == p.x && y == p.y && z == p.z && w == p.w; }
	bool operator != (const TPoint4 &p) const	{ return x != p.x || y != p.y || z != p.z || w != p.w; }
	bool operator < (const TPoint4& p) const	{ return x < p.x && y < p.y && z < p.z && w < p.w; }

	void operator += (const TPoint4 &p)		{ x += p.x; y += p.y; z += p.z; w += p.w; }
	void operator -= (const TPoint4 &p)		{ x -= p.x; y -= p.y; z -= p.z; w -= p.w; }
	void operator /= (const TPoint4 &p)		{ x /= p.x; y /= p.y; z /= p.z; w /= p.w; }

	void operator += (T s)					{ x += s; y += s; z += s; w += s; }
	void operator -= (T s)					{ x -= s; y -= s; z -= s; w -= s; }
	void operator *= (T s)					{ x *= s; y *= s; z *= s; w *= s; }
	void operator /= (T s)					{ x /= s; y /= s; z /= s; w /= s; }

	TPoint4 operator + (const TPoint4 &p) const		{ return TPoint4(x + p.x, y + p.y, z + p.z, w + p.w); }
	TPoint4 operator - (const TPoint4 &p) const		{ return TPoint4(x - p.x, y - p.y, z - p.z, w - p.w); }
	TPoint4 operator / (const TPoint4 &p) const		{ return TPoint4(x / p.x, y / p.y, z / p.z, w / p.w); }
	float operator * (const TPoint4 &p) const		{ return x * p.x + y * p.y + z * p.z + w * p.w; }

	TPoint4 operator + (T s) const		{ return TPoint4(x + s, y + s, z + s, w + s); }
	TPoint4 operator - (T s) const		{ return TPoint4(x - s, y - s, z - s, w - s); }
	TPoint4 operator * (T s) const		{ return TPoint4(x * s, y * s, z * s, w * s); }
	TPoint4 operator / (T s) const		{ return TPoint4(x / s, y / s, z / s, w / s); }

	TPoint4 operator -() const						{ return TPoint4(-x, -y, -z, -w); }

	void			componentMul(const TPoint4 &p)		{ x *= p.x; y *= p.y; z *= p.z; w *= p.w; }
	TPoint4			rComponentMul(const TPoint4 &p) const { return TPoint4(x * p.x, y * p.y, z * p.z, w * p.w); }
	inline T		sqrLength() const;
	inline T		length() const;
	inline void		normalise();
	inline TPoint4	normal() const						{ TPoint4 r(*this); r.normalise(); return r; }
};


// global operators
template <class T>
inline TPoint4<T> operator * (T s, const TPoint4<T> &p)
{
	return TPoint4<T>(p.x * s, p.y * s, p.z * s, p.w * s);
}

template <class T>
inline TPoint4<T> operator / (T s, const TPoint4<T> &p)
{
	return TPoint4<T>(p.x / s, p.y / s, p.z / s, p.w / s);
}

template <class T>
inline TPoint4<T> operator - (T s, const TPoint4<T> &p)
{
	return TPoint4<T>(p.x - s, p.y - s, p.z - s, p.w - s);
}

template <class T>
inline TPoint4<T> operator + (T s, const TPoint4<T> &p)
{
	return TPoint4<T>(p.x + s, p.y + s, p.z + s, p.w + s);
}


// sqrLength
template <class T>
inline T TPoint4<T>::sqrLength() const
{
	return x * x + y * y + z * z + w * w;
}


// length
template <class T>
inline T TPoint4<T>::length() const
{
	return (T)sqrt(sqrLength());
}


// normalise
template <class T>
inline void TPoint4<T>::normalise()
{
	T l = length();

	if (l)
		operator /= (l);
}

typedef TPoint4<float> Point4;

//-----------------------------------------------------------------------------------------------------------
// TPoint3
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TPoint3
{
public:
	typedef T Element;

	T x, y, z;

	TPoint3() {}
	TPoint3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	TPoint3(const TPoint3 &p) : x(p.x), y(p.y), z(p.z) {}
	void operator()(T _x, T _y, T _z)		{ x = _x; y = _y; z = _z; }

	STANDARD_API static const TPoint3 ZERO;
	STANDARD_API static const TPoint3 ONE;
	STANDARD_API static const TPoint3 MAX;

	operator T *()							{ return &x; }

	TPoint3& operator = (const TPoint3 &p)	{ x = p.x; y = p.y; z = p.z; return *this; }

	// index addressing
	T& operator[] (int idx)					{ return *(&x + idx); }
	const T& operator[] (int idx) const 	{ return *(&x + idx); }

	bool operator == (const TPoint3 &p)	const	{ return x == p.x && y == p.y && z == p.z; }
	bool operator != (const TPoint3 &p) const	{ return x != p.x || y != p.y || z != p.z; }
	bool operator < (const TPoint3& p) const	{ return x < p.x && y < p.y && z < p.z; }

	void operator += (const TPoint3 &p)		{ x += p.x; y += p.y; z += p.z; }
	void operator -= (const TPoint3 &p)		{ x -= p.x; y -= p.y; z -= p.z; }
	void operator /= (const TPoint3 &p)		{ x /= p.x; y /= p.y; z /= p.z; }

	void operator += (T s)					{ x += s; y += s; z += s; }
	void operator -= (T s)					{ x -= s; y -= s; z -= s; }
	void operator *= (T s)					{ x *= s; y *= s; z *= s; }
	void operator /= (T s)					{ x /= s; y /= s; z /= s; }

	TPoint3 operator + (const TPoint3 &p) const		{ return TPoint3(x + p.x, y + p.y, z + p.z); }
	TPoint3 operator - (const TPoint3 &p) const		{ return TPoint3(x - p.x, y - p.y, z - p.z); }
	TPoint3 operator / (const TPoint3 &p) const		{ return TPoint3(x / p.x, y / p.y, z / p.z); }
	float operator * (const TPoint3 &p) const		{ return x * p.x + y * p.y + z * p.z; }

	TPoint3 operator + (T s) const		{ return TPoint3(x + s, y + s, z + s); }
	TPoint3 operator - (T s) const		{ return TPoint3(x - s, y - s, z - s); }
	TPoint3 operator * (T s) const		{ return TPoint3(x * s, y * s, z * s); }
	TPoint3 operator / (T s) const		{ return TPoint3(x / s, y / s, z / s); }

	TPoint3 operator -() const						{ return TPoint3(-x, -y, -z); }

	TPoint2<T> xy() const;

	void			componentMul(const TPoint3 &p)		{ x *= p.x; y *= p.y; z *= p.z; }
	TPoint3			rComponentMul(const TPoint3 &p) const { return TPoint3(x * p.x, y * p.y, z * p.z); }
	inline void		cross(const TPoint3 &p);
	TPoint3			rCross(const TPoint3 &p) const		{ TPoint3 q(*this); q.cross(p); return q; }
	inline T		sqrLength() const;
	inline T		length() const;
	inline void		normalise();
	inline TPoint3	normal() const						{ TPoint3 r(*this); r.normalise(); return r; }
	inline void		perpVec();
};


// global operators
template <class T>
inline TPoint3<T> operator * (T s, const TPoint3<T> &p)
{
	return TPoint3<T>(p.x * s, p.y * s, p.z * s);
}

template <class T>
inline TPoint3<T> operator / (T s, const TPoint3<T> &p)
{
	return TPoint3<T>(p.x / s, p.y / s, p.z / s);
}

template <class T>
inline TPoint3<T> operator - (T s, const TPoint3<T> &p)
{
	return TPoint3<T>(p.x - s, p.y - s, p.z - s);
}

template <class T>
inline TPoint3<T> operator + (T s, const TPoint3<T> &p)
{
	return TPoint3<T>(p.x + s, p.y + s, p.z + s);
}


// cross
template <class T>
inline void TPoint3<T>::cross(const TPoint3<T> &p)
{
	T nx, ny, nz;

	nx = (y * p.z) - (z * p.y);
	ny = (z * p.x) - (x * p.z);
	nz = (x * p.y) - (y * p.x);

	x = nx;
	y = ny;
	z = nz;
}


// sqrLength
template <class T>
inline T TPoint3<T>::sqrLength() const
{
	return x * x + y * y + z * z;
}


// length
template <class T>
inline T TPoint3<T>::length() const
{
	return (T)sqrt(sqrLength());
}


// normalise
template <class T>
inline void TPoint3<T>::normalise()
{
	T l = length();

	if (l)
		operator /= (l);
}


// perpVec
template <class T>
inline void TPoint3<T>::perpVec()
{
	// http://astronomy.swin.edu.au/~pbourke/geometry/disk/
	TPoint3<T> p(*this);
	p.x += 10;
	p.y += 5;

	cross(p);
	normalise();
}


typedef TPoint3<float> Point3;


//-----------------------------------------------------------------------------------------------------------
// TPoint2
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TPoint2
{
public:
	typedef T Element;

	T x, y;

	TPoint2() {}
	TPoint2(T _x, T _y) : x(_x), y(_y) {}
	TPoint2(const TPoint2& p) : x(p.x), y(p.y) {}

	template <class O>
	operator TPoint2<O>() const { return TPoint2<O>((O)x, (O)y); }

	void operator()(T _x, T _y)				{ x = _x; y = _y; }

	STANDARD_API static const TPoint2 ZERO;
	STANDARD_API static const TPoint2 ONE;
	STANDARD_API static const TPoint2 MAX;

	operator T *()							{ return &x; }

	TPoint2& operator = (const TPoint2& p)	{ x = p.x; y = p.y; return *this; }

	template <class O> bool operator == (const TPoint2<O>& p)	const	{ return x == (T)p.x && y == (T)p.y; }
	template <class O> bool operator != (const TPoint2<O>& p)	const	{ return x != (T)p.x || y != (T)p.y; }
	template <class O> bool operator < (const TPoint2<O>& p) const	{ return x < (T)p.x && y < (T)p.y; }

	template <class O> void operator += (const TPoint2<O>& p)		{ x += (T)p.x; y += (T)p.y; }
	template <class O> void operator -= (const TPoint2<O>& p)		{ x -= (T)p.x; y -= (T)p.y; }
	template <class O> void operator *= (const TPoint2<O>& p)		{ x *= (T)p.x; y *= (T)p.y; }
	template <class O> void operator /= (const TPoint2<O>& p)		{ x /= (T)p.x; y /= (T)p.y; }

	void operator += (T s)					{ x += s; y += s; }
	void operator -= (T s)					{ x -= s; y -= s; }
	void operator *= (T s)					{ x *= s; y *= s; }
	void operator /= (T s)					{ x /= s; y /= s; }

	template <class O> TPoint2 operator + (const TPoint2<O>& p) const		{ return TPoint2(x + (T)p.x, y + (T)p.y); }
	template <class O> TPoint2 operator - (const TPoint2<O>& p) const		{ return TPoint2(x - (T)p.x, y - (T)p.y); }
	template <class O> TPoint2 operator * (const TPoint2<O>& p) const		{ return TPoint2(x * (T)p.x, y * (T)p.y); }
	template <class O> TPoint2 operator / (const TPoint2<O>& p) const		{ return TPoint2(x / (T)p.x, y / (T)p.y); }

	TPoint2 operator + (T s) const		{ return TPoint2(x + s, y + s); }
	TPoint2 operator - (T s) const		{ return TPoint2(x - s, y - s); }
	TPoint2 operator * (T s) const		{ return TPoint2(x * s, y * s); }
	TPoint2 operator / (T s) const		{ return TPoint2(x / s, y / s); }

	TPoint2 operator -() const						{ return TPoint2(-x, -y); }

	TPoint3<T> xyc(T c = (T)0.0) const;

	float			dot(const TPoint2& p) const		{ return x * p.x + y * p.y; }
	void			componentMul(const TPoint2& p)	{ x *= p.x; y *= p.y; }
	TPoint2			rComponentMul(const TPoint2& p)	const { return TPoint2(x * p.x, y * p.y); }
	inline T		sqrLength() const;
	inline T		length() const;
	inline void		normalise();
	inline TPoint2	normal() const					{ TPoint2 r(*this); r.normalise(); return r; }
};


// global operators
template <class T>
inline TPoint2<T> operator * (T s, const TPoint2<T> &p)
{
	return TPoint2<T>(p.x * s, p.y * s);
}

template <class T>
inline TPoint2<T> operator / (T s, const TPoint2<T> &p)
{
	return TPoint2<T>(p.x / s, p.y / s);
}

template <class T>
inline TPoint2<T> operator - (T s, const TPoint2<T> &p)
{
	return TPoint2<T>(p.x - s, p.y - s);
}

template <class T>
inline TPoint2<T> operator + (T s, const TPoint2<T> &p)
{
	return TPoint2<T>(p.x + s, p.y + s);
}


// sqrLength
template <class T>
inline T TPoint2<T>::sqrLength() const
{
	return x * x + y * y;
}


// length
template <class T>
inline T TPoint2<T>::length() const
{
	return (T)sqrt(sqrLength());
}


// normalise
template <class T>
inline void TPoint2<T>::normalise()
{
	operator /= (length());
}

typedef TPoint2<float> Point2;
typedef TPoint2<int> Point2i;
typedef TPoint2<uint> Point2ui;

template <class T>
TPoint3<T> TPoint2<T>::xyc(T c) const { return TPoint3<T>(x, y, c); }

template <class T>
TPoint2<T> TPoint3<T>::xy() const { return TPoint2<T>(x, y); }

//-----------------------------------------------------------------------------------------------------------
// TMatrix
// Square matrix base class
//-----------------------------------------------------------------------------------------------------------
template <class T, int D>
class TMatrix
{
public:
	TMatrix() {}
	TMatrix(T* m) { std::copy(m, m + D * D, (T*)d); }

	inline void identity();

	inline TMatrix operator - () const;
	inline TMatrix operator * (const TMatrix& rhs) const;
	inline void operator *= (const TMatrix& rhs);
	inline TMatrix operator + (const TMatrix& rhs) const { TMatrix<T, D> m(*this); m.add(rhs); return m; }
	inline TMatrix operator - (const TMatrix& rhs) const { TMatrix<T, D> m(*this); m.sub(rhs); return m; }
	inline TMatrix operator * (T rhs) const { TMatrix<T, D> m(*this); m.multiply(rhs); return m; }
	inline void operator += (const TMatrix& rhs) { add(rhs); }
	inline void operator -= (const TMatrix& rhs) { sub(rhs); }
	inline void operator *= (T rhs) { multiply(rhs); }

	operator T* () { return d[0][0]; }

	inline T& operator()(int r, int c) { return d[r][c]; }
	inline const T& operator()(int r, int c) const { return d[r][c]; }

	void clear() { memset(d, 0, sizeof(T) * D * D); }

	inline T determinant() const;
	inline T minor(int r, int c) const;
	inline T cofactor(int r, int c) const;
	inline TMatrix<T, D> cofactor() const;
	inline TMatrix<T, D> adjoint() const;
	inline bool invert();

protected:
	void copy(const TMatrix& rhs) { std::copy((T*)rhs.d, (T*)rhs.d + D * D, (T*)d); }

	T &m(int r, int c) { return d[r][c]; }
	const T &m(int r, int c) const { return d[r][c]; }

	T d[D][D];

	inline void multiply(const TMatrix& mat);
	inline void multiply(T scalar);
	inline void add(const TMatrix& mat);
	inline void sub(const TMatrix& mat);
};

// unary operator -
template <class T, int D>
inline TMatrix<T, D> TMatrix<T, D>::operator - () const
{
	TMatrix<T, D> newMat(*this);

	for (uint i = 0; i < 3; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
			newMat.d[i][j] *= -1;
		}
	}

	return newMat;
}

// operator *
template <class T, int D>
inline TMatrix<T, D> TMatrix<T, D>::operator * (const TMatrix<T, D> &mat) const
{
	TMatrix<T, D> newMat(*this);
	newMat.multiply(mat);
	return newMat;
}


// operator *=
template <class T, int D>
inline void TMatrix<T, D>::operator *= (const TMatrix<T, D> &mat)
{
	multiply(mat);
}

// identity
template <class T, int D>
inline void TMatrix<T, D>::identity()
{
	clear();
	for (int i = 0; i < D; i++)
	{
		m(i, i) = 1;
	}
}

// multiply
template <class T, int D>
inline void TMatrix<T, D>::multiply(const TMatrix<T, D> &mat)
{
	TMatrix<T, D> out;

	for (int i = 0; i < D; i++)
	{
		for (int j = 0; j < D; j++)
		{
			out.m(i, j) = m(i, 0) * mat.m(0, j);

			for (int k = 1; k < D; k++)
			{
				out.m(i, j) += m(i, k) * mat.m(k, j);
			}
		}
	}

	*this = out;
}

template <class T, int D>
inline void TMatrix<T, D>::multiply(T scalar)
{
	for (int i = 0; i < D; i++)
	{
		for (int j = 0; j < D; j++)
		{
			d[i][j] *= scalar;
		}
	}
}

template <class T, int D>
inline void TMatrix<T, D>::add(const TMatrix& mat)
{
	for (int i = 0; i < D; i++)
	{
		for (int j = 0; j < D; j++)
		{
			d[i][j] += mat.d[i][j];
		}
	}
}

template <class T, int D>
inline void TMatrix<T, D>::sub(const TMatrix& mat)
{
	for (int i = 0; i < D; i++)
	{
		for (int j = 0; j < D; j++)
		{
			d[i][j] -= mat.d[i][j];
		}
	}
}

template<class T, int D>
T TMatrix<T, D>::minor(int r, int c) const
{
	TMatrix<T, D-1> ret;

	int k=0;
	int l;

    for (int i = 0; i < D-1; ++i)
	{
		if (k == r)
			++k;
		l = 0;
		for (int j = 0; j < D-1; ++j)
		{
			if (l == c)
				++l;
			ret(i, j) = m(k, l);
			++l;
		}
		++k;
	}

	return ret.determinant();
}

template<class T, int D>
T TMatrix<T, D>::cofactor(int r, int c) const
{
	return minor(r, c) * (1.0f - (((r+1)+(c+1)) % 2) * 2.0f);
}

template<class T, int D>
TMatrix<T, D> TMatrix<T, D>::cofactor() const
{
	TMatrix<T, D> m;

    for (int i = 0; i < D; ++i)
	{
		for (int j = 0; j < D; ++j)
		{
			m(i, j) = cofactor(i, j);
		}
	}

	return m;
}

template<class T, int D>
TMatrix<T, D> TMatrix<T, D>::adjoint() const
{
	TMatrix<T, D> m;

    for (int i = 0; i < D; ++i)
	{
		for (int j = 0; j < D; ++j)
		{
			m(j, i) = cofactor(i, j);
		}
	}

	return m;
}

template<class T, int D>
T calcDeterminant(const TMatrix<T, D>& m)
{
	float ret = 0;

    for (int i = 0; i < D; ++i)
	{
		ret += m(0, i) * m.cofactor(0, i);
	}

	return ret;
}

// 2x2 matrix determinant specialization
template<class T>
T calcDeterminant(const TMatrix<T, 2>& m)
{
	return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

// 3x3 matrix determinant specialization
/*template <class T>
inline T calcDeterminant(const TMatrix<T, 3>& m)
{
	return m(0,0) * m(1,1) * m(2,2) -
			m(0,0) * m(1,2) * m(2,1) -
			m(0,1) * m(1,0) * m(2,2) +
			m(0,1) * m(1,2) * m(0,2) +
			m(0,2) * m(1,0) * m(2,1) -
			m(0,2) * m(1,1) * m(2,0);
}*/

template<class T, int D>
T TMatrix<T, D>::determinant() const
{
	return calcDeterminant(*this);
}

template<class T, int D>
bool calcInvert(TMatrix<T, D>& m)
{
	float matDet = m.determinant();
	if (matDet == 0)
		return false;

	m = m.adjoint();
	m *= (1.0f / matDet);

	return true;
}

template<class T, int D>
bool TMatrix<T, D>::invert()
{
	return calcInvert(*this);
}

//-----------------------------------------------------------------------------------------------------------
// TMatrix2
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TMatrix2 : public TMatrix<T, 2>
{
public:
	STANDARD_API static const TMatrix2 IDENTITY;

	TMatrix2() {}

	TMatrix2(const TMatrix<T, 2>& rhs)
	{
	  	TMatrix<T, 2>::copy(rhs);
	}

	TMatrix2(float r0c0, float r0c1, float r1c0, float r1c1)
	{
		this->m(0,0) = r0c0; this->m(0,1) = r0c1;
		this->m(1,0) = r1c0; this->m(1,1) = r1c1;
	}

	TMatrix2(const Point2& r1, const Point2& r2)
	{
		this->m(0,0) = r1.x; this->m(0,1) = r1.y;
		this->m(1,0) = r2.x; this->m(1,1) = r2.y;
	}
};

typedef TMatrix2<float> Matrix2;

// 2x2 matrix invert specialization
template <class T>
inline bool calcInvert(TMatrix<T, 2>& m)
{
	T det = m.determinant();
	if (det == 0)
		return false;

	TMatrix2<T> out;

	T invDet = 1.0f / det;
	out(0, 0) = invDet * m(1, 1);
	out(0, 1) = invDet * -m(0, 1);
	out(1, 0) = invDet * -m(1, 0);
	out(1, 1) = invDet * m(0, 0);

	m = out;

	return true;
}


//-----------------------------------------------------------------------------------------------------------
// TMatrix3
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TMatrix3 : public TMatrix<T, 3>
{
public:
	STANDARD_API static const TMatrix3 IDENTITY;

	TMatrix3() {}

	TMatrix3(const TMatrix<T, 3>& rhs)
	{
	  TMatrix<T, 3>::copy(rhs);
	}

	TMatrix3(const Point3& r1, const Point3& r2, const Point3& r3)
	{
		this->m(0,0) = r1.x; this->m(0,1) = r1.y; this->m(0,2) = r1.z;
		this->m(1,1) = r2.x; this->m(1,1) = r2.y; this->m(1,2) = r2.z;
		this->m(2,2) = r3.x; this->m(2,2) = r3.y; this->m(2,2) = r3.z;
	}
};

typedef TMatrix3<float> Matrix3;

//-----------------------------------------------------------------------------------------------------------
// TMatrix4
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TMatrix4 : public TMatrix<T, 4>
{
public:
	STANDARD_API static const TMatrix4 IDENTITY;

	TMatrix4() {}

	TMatrix4(const TMatrix<T, 4>& rhs)
	{
	  TMatrix<T, 4>::copy(rhs);
	}

	operator const struct _D3DMATRIX*() const { return (const struct _D3DMATRIX *)(this); }
	operator struct _D3DMATRIX*() const { return (struct _D3DMATRIX *)(this); }
	operator const struct D3DXMATRIX*() const { return (const struct D3DXMATRIX *)(this); }
	operator struct D3DXMATRIX*() const { return (struct D3DXMATRIX *)(this); }

	inline void translation(const Point3& amt) { translation(amt.x, amt.y, amt.z); }
	inline void translation(T x, T y, T z);

	inline void rotation(const Matrix3& rot);

	inline Point3 translationPart() const { return Point3(this->operator()(3, 0), this->operator()(3, 1), this->operator()(3, 2)); }
	inline Matrix3 rotationPart() const;

	inline void scale(const Point3& amt) { scale(amt.x, amt.y, amt.z); }
	inline void scale(T x, T y, T z);

	inline void transposeRotation();
	inline bool invertSimple();
	inline bool invertBlockwise();
};


typedef TMatrix4<float> Matrix4;

// translation
template <class T>
inline void TMatrix4<T>::translation(T x, T y, T z)
{
	this->m(3, 0) = x; this->m(3, 1) = y; this->m(3, 2) = z;
}

// scale
template <class T>
inline void TMatrix4<T>::scale(T x, T y, T z)
{
	this->m(0, 0) = x; this->m(1, 1) = y; this->m(2, 2) = z;
}

// transposeRotation
template <class T>
inline void TMatrix4<T>::transposeRotation()
{
	TMatrix4 out;

	for (uint i = 0; i < 3; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
    		out.m(i, j) = this->m(j, i);
		}
	}

	for (uint i = 0; i < 3; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
			this->m(i, j) = out(i, j);
		}
	}
}

namespace Det2x2
{
	template <class T>
 	struct DetFunc
	{
		float operator()(T a, T b, T c, T d)
		{
			return a * d - b * c;
		};
	}; // 2x2 determinant
}

// invertSimple
// inverts the rotation and translation part of a matrix
template <class T>
inline bool TMatrix4<T>::invertSimple()
{
	TMatrix4 out;

	Det2x2::DetFunc<T> Det;
	T determinant3x3 =  this->m(0,0) * this->m(1,1) * this->m(2,2) -
						this->m(0,0) * this->m(1,2) * this->m(2,1) -
						this->m(0,1) * this->m(1,0) * this->m(2,2) +
						this->m(0,1) * this->m(1,2) * this->m(0,2) +
						this->m(0,2) * this->m(1,0) * this->m(2,1) -
						this->m(0,2) * this->m(1,1) * this->m(2,0);

	if (determinant3x3 == 0)
		return false;

	// invert rotation/scale part
	out(0,0) = Det(this->m(1,1),this->m(1,2),this->m(2,1),this->m(2,2)); out(0,1) = Det(this->m(0,2),this->m(0,1),this->m(2,2),this->m(2,1)); out(0,2) = Det(this->m(0,1),this->m(0,2),this->m(1,1),this->m(1,2));
	out(1,0) = Det(this->m(1,2),this->m(1,0),this->m(2,2),this->m(2,0)); out(1,1) = Det(this->m(0,0),this->m(0,2),this->m(2,0),this->m(2,2)); out(1,2) = Det(this->m(0,2),this->m(0,0),this->m(1,2),this->m(1,0));
	out(2,0) = Det(this->m(1,0),this->m(1,1),this->m(2,0),this->m(2,1)); out(2,1) = Det(this->m(0,1),this->m(0,0),this->m(2,1),this->m(2,0)); out(2,2) = Det(this->m(0,0),this->m(0,1),this->m(1,0),this->m(1,1));

	out *= (T)1.0 / determinant3x3;

	// invert translation part
	for (uint i = 0; i < 3; i++)
		out(3, i) =	-(this->m(3, 0) * out(0, i)) -
		    			this->m(3, 1) * out(1, i) -
		    			this->m(3, 2) * out(2, i);

	// copy values
	for (uint i = 0; i < 4; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
			this->m(i, j) = out(i, j);
		}
	}

	return true;
}

// rotation
template <class T>
inline void TMatrix4<T>::rotation(const Matrix3& rot)
{
	for (uint i = 0; i < 3; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
			this->m(i, j) = rot(i, j);
		}
	}
}

// rotationPart
template <class T>
inline Matrix3 TMatrix4<T>::rotationPart() const
{
	Matrix3 out;

	for (uint i = 0; i < 3; i++)
	{
		for (uint j = 0; j < 3; j++)
		{
			out(i, j) = this->m(i, j);
		}
	}

	return out;
}

// invertBlockwise
template <class T>
inline bool TMatrix4<T>::invertBlockwise()
{
	TMatrix2<T> a(this->m(0, 0), this->m(0, 1), this->m(1, 0), this->m(1, 1));
	TMatrix2<T> ia(a);
	if (!ia.invert())
		return false;

	TMatrix2<T> b(this->m(0, 2), this->m(0, 3), this->m(1, 2), this->m(1, 3));
	TMatrix2<T> c(this->m(2, 0), this->m(2, 1), this->m(3, 0), this->m(3, 1));
	TMatrix2<T> d(this->m(2, 2), this->m(2, 3), this->m(3, 2), this->m(3, 3));

	TMatrix2<T> invSchur(c);
	invSchur *= ia;
	invSchur *= b;
	invSchur *= -1.0f;
	invSchur += d;
	if (!invSchur.invert())
		return false;

	TMatrix2<T> iaxbxinvSchur(ia);
	iaxbxinvSchur *= b;
	iaxbxinvSchur *= invSchur;

	TMatrix2<T> cxia(c);
	cxia *= ia;

	// outa
	a = iaxbxinvSchur;
	a *= cxia;
	a += ia;

	// outb
	b = iaxbxinvSchur;
	b *= -1.0f;

	// outc
	c = invSchur;
	c *= cxia;
	c *= -1.0f;

	// outd
	d = invSchur;

	this->m(0, 0) = a(0, 0);
	this->m(0, 1) = a(0, 1);
	this->m(1, 0) = a(1, 0);
	this->m(1, 1) = a(1, 1);
	this->m(0, 2) = b(0, 0);
	this->m(0, 3) = b(0, 1);
	this->m(1, 2) = b(1, 0);
	this->m(1, 3) = b(1, 1);
	this->m(2, 0) = c(0, 0);
	this->m(2, 1) = c(0, 1);
	this->m(3, 0) = c(1, 0);
	this->m(3, 1) = c(1, 1);
	this->m(2, 2) = d(0, 0);
	this->m(2, 3) = d(0, 1);
	this->m(3, 2) = d(1, 0);
	this->m(3, 3) = d(1, 1);

	return true;
}

//-----------------------------------------------------------------------------------------------------------
// TQuat
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TQuat
{
public:
	float w;
	TPoint3<T> a;

	TQuat() { }
	TQuat(T _w, const TPoint3<T>& _a) : w(_w), a(_a) {}
	TQuat(T _w, T _ax, T _ay, T _az) : w(_w), a(_ax, _ay, _az) {}
	void operator()(T _w, const TPoint3<T>& _a) { w = _w; a = _a; }

	STANDARD_API static const TQuat IDENTITY;
	STANDARD_API static const TQuat NONE;

	bool operator == (const TQuat<T> &p) const	{ return w == p.w && a == p.a; }
	bool operator != (const TQuat<T> &p) const	{ return w != p.w || a != p.a; }

	TQuat<T> operator * (const TQuat<T>& rhs) const { TQuat<T> q(*this); q.multiply(rhs); return q; }
	TQuat<T> operator * (float rhs) const { TQuat<T> q(*this); q.a = a * rhs; q.w = w * rhs; return q; }
	void operator *= (const TQuat<T>& rhs) { multiply(rhs); }
	void operator += (const TQuat<T>& rhs) { w += rhs.w; a += rhs.a; }
	TQuat<T> operator - () { TQuat<T> q(*this); q.invert(); return q; }
	TQuat<T> operator + (const TQuat<T>& rhs) const { TQuat<T> q(*this); q.w += rhs.w; q.a += rhs.a; return q; }
	TQuat<T> operator - (const TQuat<T>& rhs) const { TQuat<T> q(*this); q.w -= rhs.w; q.a -= rhs.a; return q; }

	TPoint3<T> operator * (TPoint3<T> p) const { multiply(p); return p; }

	inline void multiply(const TQuat<T>& quat);
	inline T dot(const TQuat<T>& quat) const;
	inline void multiply(TPoint3<T>& p) const;
	inline void angleAxis(T angle, const TPoint3<T>& axis);
	inline void toMatrix(Matrix3& mat) const;
	inline void toMatrix(Matrix4& mat)const;
	inline void fromMatrix(Matrix3& mat);
	inline void fromZYX(const Point3& p);
	inline void normalise();
	inline void invert();
	inline void identity() { w = 1; a = Point3(0, 0, 0); }
	inline T sqrLength() const;
	inline TPoint3<T> vec(const TPoint3<T>& identityDir) const;
};

typedef TQuat<float> Quat;

// Quaternion global functions
template <class T>
inline TQuat<T> QuatAngleAxis(T angle, const TPoint3<T>& axis)
{
	TQuat<T> q;
	q.angleAxis(angle, axis);
	return q;
}

template <class T>
inline TQuat<T> QuatFromVector(const TPoint3<T>& identity, const TPoint3<T>& target, const TPoint3<T>& perpPlane = TPoint3<T>(0, 0, 1))
{
	float dot = target * identity;
	if (dot == 0.0f)
	{
		return Quat::IDENTITY;
	}

	Point3 cross;
	if (abs(dot) == 1.0f)
	{
		cross = perpPlane;
	}
	else
	{
		cross = identity;
		cross.cross(target);
		cross.normalise();
	}

	return QuatAngleAxis<T>(acos(dot), cross);
}


// multiply
template <class T>
void TQuat<T>::multiply(const TQuat<T> &quat)
{
	float nw = (w * quat.w) - (a * quat.a);
	TPoint3<T> na = a.rCross(quat.a) + (w * quat.a) + (quat.w * a);

	w = nw;
	a = na;
}


// multiply
template <class T>
void TQuat<T>::multiply(TPoint3<T> &p) const
{
	TQuat<T> q(0, p);
	TQuat<T> c(w, -a);
	TQuat<T> rot = *this;

	rot.multiply(q);
	rot.multiply(c);

	p = rot.a;
}


// dot
template <class T>
T TQuat<T>::dot(const TQuat<T>& quat) const
{
	return w * quat.w + a * quat.a;
}


// angleAxis
template <class T>
void TQuat<T>::angleAxis(T angle, const TPoint3<T> &axis)
{
	angle *= 0.5;

	w = (T)cos(angle);
	a = axis;
	a *= (T)sin(angle);

	normalise();
}


// toMatrix
template <class T>
void TQuat<T>::toMatrix(Matrix3 &mat) const
{
	float xx = this->sqr(a.x);
	float yy = this->sqr(a.y);
	float zz = this->sqr(a.z);
	float ww = this->sqr(w);
	float xy = a.x * a.y;
	float xz = a.x * a.z;
	float yz = a.y * a.z;
	float wz = w * a.z;
	float wy = w * a.y;
	float wx = w * a.x;

    mat(0, 0) = 1 - 2 * ( yy + zz );
    mat(0, 1) =     2 * ( xy - wz );
    mat(0, 2) =     2 * ( xz + wy );
    mat(1, 0) =     2 * ( xy + wz );
    mat(1, 1) = 1 - 2 * ( xx + zz );
    mat(1, 2) =     2 * ( yz - wx );
    mat(2, 0) =     2 * ( xz - wy );
    mat(2, 1) =     2 * ( yz + wx );
    mat(2, 2) = 1 - 2 * ( xx + yy );
}


// toMatrix
template <class T>
void TQuat<T>::toMatrix(Matrix4 &mat) const
{
	float xx = a.x * a.x;
	float yy = a.y * a.y;
	float zz = a.z * a.z;
	float xy = a.x * a.y;
	float xz = a.x * a.z;
	float yz = a.y * a.z;
	float wz = w * a.z;
	float wy = w * a.y;
	float wx = w * a.x;

    mat(0, 0) = 1 - 2 * ( yy + zz );
    mat(0, 1) =     2 * ( xy + wz );
    mat(0, 2) =     2 * ( xz - wy );
    mat(1, 0) =     2 * ( xy - wz );
    mat(1, 1) = 1 - 2 * ( xx + zz );
    mat(1, 2) =     2 * ( yz + wx );
    mat(2, 0) =     2 * ( xz + wy );
    mat(2, 1) =     2 * ( yz - wx );
    mat(2, 2) = 1 - 2 * ( xx + yy );

	mat(0, 3) = 0;
	mat(1, 3) = 0;
	mat(2, 3) = 0;
	mat(3, 0) = 0;
	mat(3, 1) = 0;
	mat(3, 2) = 0;
	mat(3, 3) = 1;
}


// fromMatrix
template <class T>
void TQuat<T>::fromMatrix(Matrix3 &mat)
{
	assert((mat(0, 0) + mat(1, 1) + mat(2, 2) + 1) / 4 > 0);
	w = (T)sqrt((mat(0, 0) + mat(1, 1) + mat(2, 2) + 1) / 4);
	a.x = (mat(2, 1) - mat(1, 2));
	a.y = (mat(0, 2) - mat(2, 0));
	a.z = (mat(1, 0) - mat(0, 1));
	a /= 4 * w;
}


// sqrLength
template <class T>
T TQuat<T>::sqrLength() const
{
	return (T)sqrt(w * w + a.sqrLength());
}


// normalise
template <class T>
void TQuat<T>::normalise()
{
	T length = sqrLength();
	w /= length;
	a /= length;
}


// invert
template <class T>
void TQuat<T>::invert()
{
	a *= -1;
}

template <class T>
TPoint3<T> TQuat<T>::vec(const TPoint3<T>& identityDir) const
{
	return identityDir * *this;
}

template <class T>
void TQuat<T>::fromZYX(const Point3& p)
{
	*this *= QuatAngleAxis(p.z, Point3(0, 0, 1));
	*this *= QuatAngleAxis(p.y, Point3(0, 1, 0));
	*this *= QuatAngleAxis(p.x, Point3(1, 0, 0));
}


//-----------------------------------------------------------------------------------------------------------
// TPlane
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TPlane
{
public:
	TPoint3<T> n;
	float d;

	TPlane() {}
	TPlane(const TPoint3<T> &normal, const TPoint3<T> &point) : n(normal), d(point * normal) {}
	TPlane(const TPoint3<T> &_n, float _d) : n(_n), d(_d) {}

	void set(const TPoint3<T> &normal, const TPoint3<T> &point)
	{
		n = normal;
		d = -point * n;
	}

	void set(const TPoint3<T> &normal, float d)
	{
		n = normal;
		d = d;
	}

	// Returns the target point on the plane that is found by following the vector along the plane's normal
	// with length equal to the distance to the plane.
	TPoint3<T> pointOnPlane(const TPoint3<T> &origin)
	{
		return origin - n * distance(origin);
	}

	inline T distance(const TPoint3<T> &p) const;
};

typedef TPlane<float> Plane;


// distance
template <class T>
T TPlane<T>::distance(const TPoint3<T> &p) const
{
	return n * p - d;
}


//-----------------------------------------------------------------------------------------------------------
// TFace
//-----------------------------------------------------------------------------------------------------------
template <uint T>
struct TFace
{
	TFace() {}
	TFace(ushort a, ushort b, ushort c) { (*((&*idx)+0)) = a; (*((&*idx)+1)) = b; (*((&*idx)+2)) = c; }

	ushort idx[T];

	ushort &operator [](int i) { return idx[i]; }
	const ushort &operator [](int i) const { return idx[i]; }
};

typedef TFace<3> Face;

//-----------------------------------------------------------------------------------------------------------
// TPoly
//-----------------------------------------------------------------------------------------------------------
template <class T, int N>
struct TPoly
{
	TPoly() {}

	TPoly(const TPoint3<T>& v0, const TPoint3<T>& v1, const TPoint3<T>& v2)
	{
		verts[0] = v0;
		verts[1] = v1;
		verts[2] = v2;
	}

	TPoly(const TPoint3<T>& v0, const TPoint3<T>& v1, const TPoint3<T>& v2, const TPoint3<T>& v3)
	{
		verts[0] = v0;
		verts[1] = v1;
		verts[2] = v2;
		verts[3] = v3;
	}

	TPoint3<T> verts[N];

	TPoint3<T>& operator [](int i) { return verts[i]; }
	const TPoint3<T>& operator [](int i) const { return verts[i]; }

	TPoint3<T> calcNormal() const
	{
		Point3 normal = verts[0] - verts[1];
		Point3 v = verts[0] - verts[2];
		v.cross(normal);
		v.normalise();

		return v;
	}

	void expand(T amt)
	{
		Point3 center(0, 0, 0);

		for (int i = 0; i < N; i++)
			center += verts[i];

		center /= N;

		for (int i = 0; i < N; i++)
		{
			verts[i] += (verts[i] - center) * amt;
		}
	}

	void translate(const TPoint3<T>& p)
	{
		for (int i = 0; i < N; i++)
		{
			verts[i] += p;
		}
	}
};

typedef TPoly<float, 3> Tri;
typedef TPoly<float, 4> Quad;


//-----------------------------------------------------------------------------------------------------------
// RGBA
//-----------------------------------------------------------------------------------------------------------
template<class T>
class TRGBA
{
public:
	TRGBA()
	{
	}

	TRGBA(T rp, T gp, T bp, T ap = 1.0f) : r(rp), b(bp), g(gp), a(ap)
	{
	}

	TRGBA(const D3DCOLORVALUE& c) : r(c.r), b(c.b), g(c.g), a(c.a)
	{
	}

	// D3DCOLOR cast
	operator uint() const { return ((uint)(((((uint)(a*255.f))&0xff)<<24)|((((uint)(r*255.f))&0xff)<<16)|((((uint)(g*255.f))&0xff)<<8)|(((uint)(b*255.f))&0xff))); }
	operator D3DCOLORVALUE() const { D3DCOLORVALUE c; c.r = r; c.g = g; c.b = b; c.a = a; return c; }

	void clamp()
	{
		::clamp(r);
		::clamp(g);
		::clamp(b);
		::clamp(a);
	}

	T luminance()
	{
		return r * 0.299f + g * 0.587f + b * 0.114f;
	}

	void grey()
	{
		r = g = b = luminance();
	}

	T r, g, b, a;
};

typedef TRGBA<float> RGBA;

//-----------------------------------------------------------------------------------------------------------
// TOBB
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TOBB
{
public:
	TOBB() {}
	TOBB(const TPoint3<T>& _origin, const TPoint3<T>& _extent, const TQuat<T>& _rot) :
		origin(_origin),
		extent(_extent),
		rot(_rot)
	{}

	TPoint3<T>	origin;
	TPoint3<T>	extent;
	TQuat<T>	rot;
};

typedef TOBB<float> OBB;

//-----------------------------------------------------------------------------------------------------------
// TAABB
// Axially-aligned bounding box
// "extent" covers the entire extent of the box
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TAABB
{
public:
	TAABB() {}
	TAABB(const TPoint3<T>& _origin, const TPoint3<T>& _extent) :
		origin(_origin),
		extent(_extent)
	{}

	TPoint3<T>	origin;
	TPoint3<T>	extent;

	// form planes from cube sides
	void toPlanes(TPlane<T> planes[]) const
	{
		static const TPoint3<T> a[6] = {
			Point3(0, 1, 0),
			Point3(0, -1, 0),
			Point3(1, 0, 0),
			Point3(-1, 0, 0),
			Point3(0, 0, 1),
			Point3(0, 0, -1)
		};

		for (int i = 0; i < 6; ++i)
		{
			planes[i] = TPlane<T>(a[i], a[i] * extent * 0.5f);
		}
	}
};

typedef TAABB<float> AABB;

//-----------------------------------------------------------------------------------------------------------
// Interpolation
//-----------------------------------------------------------------------------------------------------------

inline float Interpolate(float time, float start, float target)
{
	assert(time >= 0 && time <= 1);

	return start + (target - start) * time;
}

inline void Interpolate(float time, const Point3& start, const Point3& target, Point3& var)
{
	assert(time >= 0 && time <= 1);

	var = start + (target - start) * time;
}

inline void Interpolate(float time, const Quat& start, const Quat& target, Quat& var)
{
	float a, s1, s2;

	float dot = start.dot(target);
	if (dot >= 1.0f)
	{
		var = start;
		return;
	}

	a = (float)::acos(dot);
	s1 = (float)sin((1-time)*a)/(float)sin(a);
	s2 = (float)sin(time*a)/(float)sin(a);

	var = start * s1;
	var += target * s2;
}


/*template <class T>
class InterpVar
{
public:
	InterpVar() : m_time(0) {}

	T& operator* ()					{ return m_target; }
	T& operator= (const T& target)	{ m_target = target; }
	T& operator+= (const T& target)	{ m_target += target; }
	T& operator-= (const T& target)	{ m_target -= target; }
	T& operator/= (const T& target)	{ m_target /= target; }
	T& operator*= (const T& target)	{ m_target *= target; }
	T& operator+ (const T& target)	{ return m_target + target; }
	T& operator- (const T& target)	{ return m_target - target; }
	T& operator/ (const T& target)	{ return m_target / target; }
	T& operator* (const T& target)	{ return m_target * target; }
	void set(const T& amt)			{ m_start = amt; m_var = amt; m_target = amt; }
	T& val()						{ return m_var; }

	void update(float delta, float length)
	{
		assert(length);

		m_time += delta;
		if (m_time > 1)
			m_time = 1;

		Interpolate(m_time, length, m_start, m_target, m_var);
	}

private:
	T m_start;
	T m_target;
	T m_var;
	float m_time;
};*/

//-----------------------------------------------------------------------------------------------------------
// TRange
//-----------------------------------------------------------------------------------------------------------

template <class T>
class TRange
{
public:
	TRange() {};
	TRange(T& _min, T& _max) { min = _min; max = _max; }

	T min, max;

	T length() { assert(max > min); return max - min; }
};

typedef TRange<int> IntRange;

//-----------------------------------------------------------------------------------------------------------
// Global operators
//-----------------------------------------------------------------------------------------------------------
// TPoint3
template <class T>
inline TPoint3<T> operator * (const TPoint3<T>& lhs, const TQuat<T>& rhs)
{
	TPoint3<T> t(lhs);

	rhs.multiply(t);
	return t;
}

template <class T>
inline void operator *= (TPoint3<T>& lhs, const TQuat<T>& rhs)
{
	rhs.multiply(lhs);
}

// TPoint3 * TMatrix
template <class T>
inline void operator *= (TPoint3<T>& lhs, const TMatrix<T, 3>& rhs)
{
	TPoint3<T> newp(lhs);

	newp.x = lhs.x * rhs(0, 0) + lhs.y * rhs(1, 0) + lhs.z * rhs(2, 0);
	newp.y = lhs.x * rhs(0, 1) + lhs.y * rhs(1, 1) + lhs.z * rhs(2, 1);
	newp.z = lhs.x * rhs(0, 2) + lhs.y * rhs(1, 2) + lhs.z * rhs(2, 2);

	lhs = newp;
}

template <class T>
inline TPoint3<T> operator * (const TPoint3<T>& lhs, const TMatrix<T, 3>& rhs)
{
	TPoint3<T> newp(lhs);

	newp *= rhs;

	return newp;
}

template <class T>
inline void operator *= (TPoint3<T>& lhs, const TMatrix<T, 4>& rhs)
{
	TPoint3<T> newp(lhs);

	newp.x = lhs.x * rhs(0, 0) + lhs.y * rhs(1, 0) + lhs.z * rhs(2, 0) + rhs(3, 0);
	newp.y = lhs.x * rhs(0, 1) + lhs.y * rhs(1, 1) + lhs.z * rhs(2, 1) + rhs(3, 1);
	newp.z = lhs.x * rhs(0, 2) + lhs.y * rhs(1, 2) + lhs.z * rhs(2, 2) + rhs(3, 2);

	lhs = newp;
}

template <class T>
inline TPoint3<T> operator * (const TPoint3<T>& lhs, const TMatrix<T, 4>& rhs)
{
	TPoint3<T> newp(lhs);

	newp *= rhs;

	return newp;
}

// TMatrix * TPoint3
template <class T>
inline TPoint3<T> operator * (const TMatrix<T, 3>& lhs, const TPoint3<T>& rhs)
{
	TPoint3<T> newp(rhs);

	newp.x = rhs.x * lhs(0, 0) + rhs.y * lhs(0, 1) + rhs.z * lhs(0, 2);
	newp.y = rhs.x * lhs(1, 0) + rhs.y * lhs(1, 1) + rhs.z * lhs(1, 2);
	newp.z = rhs.x * lhs(2, 0) + rhs.y * lhs(2, 1) + rhs.z * lhs(2, 2);

	return newp;
}

template <class T>
inline TPoint3<T> operator * (const TMatrix<T, 4>& lhs, const TPoint3<T>& rhs)
{
	TPoint3<T> newp(rhs);

	newp.x = rhs.x * lhs(0, 0) + rhs.y * lhs(0, 1) + rhs.z * lhs(0, 2) + lhs(0, 3);
	newp.y = rhs.x * lhs(1, 0) + rhs.y * lhs(1, 1) + rhs.z * lhs(1, 2) + lhs(1, 3);
	newp.z = rhs.x * lhs(2, 0) + rhs.y * lhs(2, 1) + rhs.z * lhs(2, 2) + lhs(2, 3);

	return newp;
}

// TMatrix4 * Point4
template <class T>
inline void operator *= (TPoint4<T>& lhs, const TMatrix<T, 4>& rhs)
{
	TPoint4<T> newp(lhs);

	newp.x = lhs.x * rhs(0, 0) + lhs.y * rhs(1, 0) + lhs.z * rhs(2, 0) + lhs.w * rhs(3, 0);
	newp.y = lhs.x * rhs(0, 1) + lhs.y * rhs(1, 1) + lhs.z * rhs(2, 1) + lhs.w * rhs(3, 1);
	newp.z = lhs.x * rhs(0, 2) + lhs.y * rhs(1, 2) + lhs.z * rhs(2, 2) + lhs.w * rhs(3, 2);
	newp.w = lhs.x * rhs(0, 3) + lhs.y * rhs(1, 3) + lhs.z * rhs(2, 3) + lhs.w * rhs(3, 3);

	lhs = newp;
}

template <class T>
inline void operator * (const TPoint4<T>& lhs, const TMatrix<T, 4>& rhs)
{
	TPoint4<T> newp(lhs);

	newp *= rhs;

	return newp;
}

template <class T>
inline void operator * (const TMatrix<T, 4>& lhs, const TPoint4<T>& rhs)
{
	TPoint4<T> newp(rhs);

	newp.x = rhs.x * lhs(0, 0) + rhs.y * lhs(0, 1) + rhs.z * lhs(0, 2) + rhs.w * lhs(0, 3);
	newp.y = rhs.x * lhs(1, 0) + rhs.y * lhs(1, 1) + rhs.z * lhs(1, 2) + rhs.w * lhs(1, 3);
	newp.z = rhs.x * lhs(2, 0) + rhs.y * lhs(2, 1) + rhs.z * lhs(2, 2) + rhs.w * lhs(2, 3);
	newp.z = rhs.x * lhs(3, 0) + rhs.y * lhs(3, 1) + rhs.z * lhs(3, 2) + rhs.w * lhs(3, 3);

	return newp;
}

//-----------------------------------------------------------------------------------------------------------
// Global functions
//-----------------------------------------------------------------------------------------------------------

template <class T>
bool pointInAABB(const TPoint3<T> &p, const TPoint3<T> &min, const TPoint3<T> &max)
{
	return p.x >= min.x && p.y >= min.y && p.z >= min.z &&
			p.x <= max.x && p.y <= max.y && p.z <= max.z;
}

template<class T> T sqr(T s) { return s*s; }

inline bool pointBox(const Point2 &p, const Point2 &min0, const Point2 &max0)
{
	return p.x >= min0.x && p.x <= max0.x && p.y >= min0.y && p.y <= max0.y;
}

}

#endif
