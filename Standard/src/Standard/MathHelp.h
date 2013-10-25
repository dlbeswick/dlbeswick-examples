#pragma once

#include "Standard/api.h"
#include "Math.h"

namespace MathHelp
{

using namespace DMath;

// minPoint
template <class T>
inline TPoint3<T> minPoint(const TPoint3<T>* start, const TPoint3<T>* end)
{
	Point3 p(FLT_MAX, FLT_MAX, FLT_MAX);

	for (; start != end; ++start)
	{
		if (start->x < p.x)
			p.x = start->x;

		if (start->y < p.y)
			p.y = start->y;

		if (start->z < p.z)
			p.z = start->z;
	}

	return p;
}

// maxPoint
template <class T>
inline TPoint3<T> maxPoint(const TPoint3<T>* start, const TPoint3<T>* end)
{
	TPoint3<T> p(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (; start != end; ++start)
	{
		if (start->x > p.x)
			p.x = start->x;

		if (start->y > p.y)
			p.y = start->y;

		if (start->z > p.z)
			p.z = start->z;
	}

	return p;
}

template <class T>
inline T max(const TPoint3<T>& p)
{
	return std::max(p.x, std::max(p.y, p.z));
}

template <class T, class It>
inline T average(It begin, const It& end)
{
	double ret = 0;
	int size = 0;

	while (begin != end)
	{
		ret += *begin++;
		++size;
	}

	return (T)(ret / size);
}

template <class T, class STL>
inline T average(const STL& data)
{
	return average<T, STL::const_iterator>(data.begin(), data.end());
}

}
