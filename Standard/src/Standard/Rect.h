#ifndef STANDARD_DRect_H
#define STANDARD_DRect_H

#include "Standard/api.h"
#include "Standard/Math.h"

namespace David
{
	class STANDARD_API Rect
	{
	public:
		Rect();
		Rect(const DMath::Point2& min, const DMath::Point2& max);

		DMath::Point2 extent() const;
		const DMath::Point2& min() const { return _min; }
		const DMath::Point2& max() const { return _max; }

		David::Rect intersection(const David::Rect& r) const;

		bool valid() const;

	#if IS_WINDOWS
		operator RECT() const;
	#endif

		void operator *= (const DMath::Point2& rhs);

	protected:
		DMath::Point2 _min;
		DMath::Point2 _max;
	};
}

#endif
