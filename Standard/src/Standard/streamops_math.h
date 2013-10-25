#ifndef STANDARD_STREAMOPS_MATH_H
#define STANDARD_STREAMOPS_MATH_H

#include "Standard/api.h"
#include "Math.h"

// tbd: remove this, qualify names
using namespace DMath;

// MATH
// Point2
inline otextstream& operator << (otextstream& s, const Point2& obj)
{
	s.s() << "(" << obj.x << ", " << obj.y << ")";

	return s;
}

inline itextstream& operator >> (itextstream& s, Point2& obj)
{
	s >> "(" >> obj.x >> "," >> obj.y >> ")";

	return s;
}

// Point2i
inline otextstream& operator << (otextstream& s, const Point2i& obj)
{
	s.s() << "(" << obj.x << ", " << obj.y << ")";

	return s;
}

inline itextstream& operator >> (itextstream& s, Point2i& obj)
{
	s >> "(" >> obj.x >> "," >> obj.y >> ")";

	return s;
}

// Point3
inline otextstream& operator << (otextstream& s, const Point3& obj)
{
	s.s() << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";

	return s;
}

inline itextstream& operator >> (itextstream& s, Point3& obj)
{
	s >> "(" >> obj.x >> "," >> obj.y >> "," >> obj.z >> ")";

	return s;
}

// Quat
inline otextstream& operator << (otextstream& s, const Quat& obj)
{
	s.s() << "(" << obj.w << ", " << obj.a.x << ", " << obj.a.y << ", " << obj.a.z << ")";

	return s;
}

inline itextstream& operator >> (itextstream& s, Quat& obj)
{
	s >> "(" >> obj.w >> "," >> obj.a.x >> "," >> obj.a.y >> "," >> obj.a.z >> ")";

	return s;
}

// RGBA
inline otextstream& operator << (otextstream& s, const RGBA& obj)
{
	s.s() << "(" << obj.r << ", " << obj.g << ", " << obj.b << ", " << obj.a << ")";

	return s;
}

inline itextstream& operator >> (itextstream& s, RGBA& obj)
{
	s >> "(" >> obj.r >> "," >> obj.g >> "," >> obj.b >> "," >> obj.a >> ")";

	return s;
}

template<class T, int D>
inline otextstream& operator << (otextstream& s, const TMatrix<T, D>& obj)
{
	s.s() << "(";

	for (int i = 0; i < D; ++i)
	{
		for (int j = 0; j < D; ++j)
		{
			s.s() << obj(i, j);
			if (i != D-1 && j != D-1)
				s.s() << ", ";
		}
	}

	s.s() << ")";

	return s;
}

template<class T, int D>
inline itextstream& operator >> (itextstream& s, TMatrix<T, D>& obj)
{
	s >> "(";

	for (int i = 0; i < D; ++i)
	{
		for (int j = 0; j < D; ++j)
		{
			s >> obj(i, j);
			if (i != D-1 && j != D-1)
				s >> ", ";
		}
	}

	s >> ")";

	return s;
}

inline otextstream& operator << (otextstream& s, const Matrix4& obj)
{
	s.s() << "(" <<
		obj(0, 0) << ", " << obj(0, 1) << ", " << obj(0, 2) << ", " << obj(0, 3) << ", " <<
		obj(1, 0) << ", " << obj(1, 1) << ", " << obj(1, 2) << ", " << obj(1, 3) << ", " <<
		obj(2, 0) << ", " << obj(2, 1) << ", " << obj(2, 2) << ", " << obj(2, 3) << ", " <<
		obj(3, 0) << ", " << obj(3, 1) << ", " << obj(3, 2) << ", " << obj(3, 3) <<
	")";

	return s;
}

inline itextstream& operator >> (itextstream& s, Matrix4& obj)
{
	s >> "(" >>
		obj(0, 0) >> ", " >> obj(0, 1) >> ", " >> obj(0, 2) >> ", " >> obj(0, 3) >> ", " >>
		obj(1, 0) >> ", " >> obj(1, 1) >> ", " >> obj(1, 2) >> ", " >> obj(1, 3) >> ", " >>
		obj(2, 0) >> ", " >> obj(2, 1) >> ", " >> obj(2, 2) >> ", " >> obj(2, 3) >> ", " >>
		obj(3, 0) >> ", " >> obj(3, 1) >> ", " >> obj(3, 2) >> ", " >> obj(3, 3) >>
	")";

	return s;
}

// MATH
// Point2
inline obinstream& operator << (obinstream& s, const Point2& obj)
{
	s.s().write((char*)&obj, sizeof(Point2));

	return s;
}

inline ibinstream& operator >> (ibinstream& s, Point2& obj)
{
	s.s().read((char*)&obj, sizeof(Point2));

	return s;
}

// Point2i
inline obinstream& operator << (obinstream& s, const Point2i& obj)
{
	s.s().write((char*)&obj, sizeof(Point2i));

	return s;
}

inline ibinstream& operator >> (ibinstream& s, Point2i& obj)
{
	s.s().read((char*)&obj, sizeof(Point2i));

	return s;
}

// Point3
inline obinstream& operator << (obinstream& s, const Point3& obj)
{
	s.s().write((char*)&obj, sizeof(Point3));

	return s;
}

inline ibinstream& operator >> (ibinstream& s, Point3& obj)
{
	s.s().read((char*)&obj, sizeof(Point3));

	return s;
}

// Quat
inline obinstream& operator << (obinstream& s, const Quat& obj)
{
	s.s().write((char*)&obj, sizeof(Quat));

	return s;
}

inline ibinstream& operator >> (ibinstream& s, Quat& obj)
{
	s.s().read((char*)&obj, sizeof(Quat));

	return s;
}

// RGBA
inline obinstream& operator << (obinstream& s, const RGBA& obj)
{
	s.s().write((char*)&obj, sizeof(RGBA));

	return s;
}

inline ibinstream& operator >> (ibinstream& s, RGBA& obj)
{
	s.s().read((char*)&obj, sizeof(RGBA));

	return s;
}

#endif
