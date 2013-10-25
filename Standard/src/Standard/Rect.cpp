#include "Standard/pch.h"
#include "Standard/Rect.h"
#include "Standard/Range.h"

using namespace DMath;

David::Rect::Rect() :
	_min(Point2::ZERO),
	_max(Point2::ZERO)
{
}

David::Rect::Rect(const Point2& min, const Point2& max) :
	_min(min),
	_max(max)
{
}

David::Rect David::Rect::intersection(const Rect& r) const
{
	Range<float> x0(_min.x, _max.x);
	Range<float> x1(r._min.x, r._max.x);
	Range<float> xIntersect = x0.intersection(x1);

	Range<float> y0(_min.y, _max.y);
	Range<float> y1(r._min.y, r._max.y);
	Range<float> yIntersect = y0.intersection(y1);

	if (xIntersect.valid() && yIntersect.valid())
	{
		return Rect(
			Point2(xIntersect.min(), yIntersect.min()),
			Point2(xIntersect.max(), yIntersect.max())
		);
	}
	else
	{
		return Rect(Point2::MAX, Point2::MAX);
	}
}

#if IS_WINDOWS
David::Rect::operator RECT() const
{
	RECT result;

	result.bottom = (int)_max.y;
	result.left = (int)_min.x;
	result.right = (int)_max.x;
	result.bottom = (int)_max.y;

	return result;
}
#endif

Point2 David::Rect::extent() const
{
	return _max - _min;
}

void David::Rect::operator *= (const Point2& rhs)
{
	_min *= rhs;
	_max *= rhs;
}

bool David::Rect::valid() const
{
	return _min != Point2::MAX && _max != Point2::MAX;
}
