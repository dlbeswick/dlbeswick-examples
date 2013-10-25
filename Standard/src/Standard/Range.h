// ------------------------------------------------------------------------------------------------
//
// Range
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Standard/Interpolation.h"
#include "Standard/Rand.h"
#include "Standard/streamops.h"

class otextstream;
class itextstream;
class obinstream;
class ibinstream;

template <typename T, typename Scalar = float>
class Range
{
public:
	Range(const T& min = T(), const T& max = T(), const Interpolation::Adapter<T, Scalar>& func = (Interpolation::Adapter<T, Scalar>(Interpolation::linear)) ) :
	  m_min(min),
	  m_max(max),
	  m_func(func)
	{}

	bool operator == (const Range<T, Scalar>& rhs) const { return m_min == rhs.m_min && m_max == rhs.m_max; }
	bool operator != (const Range<T, Scalar>& rhs) const { return !operator==(rhs); }

	T& min() { return m_min; }
	const T& min() const { return m_min; }

	T& max() { return m_max; }
	const T& max() const { return m_max; }

	T value(Scalar u, const Interpolation::Adapter<T, Scalar>& func) const { return func(m_min, m_max, u); }

	T operator() (Scalar u) const { return value(u, m_func); }

	T operator() () const { return value(Rand(), m_func); }

	Range intersection(const Range& r) const;

	bool valid() const { return m_min != FLT_MAX && m_max != FLT_MAX; }

protected:
	T m_min;
	T m_max;
	Interpolation::Adapter<T, Scalar> m_func;

	// tbd: separate streaming code from class
	friend otextstream& operator << (otextstream& s, const Range<T, Scalar>& obj)
	{
		s << "(" << obj.m_min << " -> " << obj.m_max << ")";

		return s;
	}

	friend itextstream& operator >> (itextstream& s, Range<T, Scalar>& obj)
	{
		s >> "(" >> obj.m_min;
		itextstream::whitespace oldWhitespace = s.m_whitespace;
		s << itextstream::whitespace();
		s >> "->";
		s.skipWhitespace();
		s << oldWhitespace;
		s >> obj.m_max >> ")";

		return s;
	}

	friend obinstream& operator << (obinstream& s, const Range<T, Scalar>& obj)
	{
		s << obj.m_min << obj.m_max;

		return s;
	}

	friend ibinstream& operator >> (ibinstream& s, Range<T, Scalar>& obj)
	{
		s >> obj.m_min >> obj.m_max;

		return s;
	}
};

template <class T, class Scalar>
Range<T, Scalar> Range<T, Scalar>::intersection(const Range<T, Scalar>& r) const
{
	float min_of_maxs = std::min(m_max, r.m_max);
    float max_of_mins = std::max(m_min, r.m_min);

	if (max_of_mins > min_of_maxs)
		return Range(FLT_MAX, FLT_MAX);
	else
		return Range(max_of_mins, min_of_maxs);
}
