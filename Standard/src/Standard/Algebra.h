// ------------------------------------------------------------------------------------------------
//
// Algebra
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/api.h"
#include "Math.h"
#include <limits>

namespace Algebra
{
	using namespace DMath;

	// Return true if r1 and r2 are real (y=ax^2 + bx + c)
	// http://www.gamasutra.com/features/19991018/Gomez_2.htm
	template <class T>
	bool solveQuadratic(const T a, const T b, const T c, T& r1, T& r2)
	{
		const T q = b*b - 4*a*c;
		if( q >= 0 )
		{
			const T sq = sqrt(q);
			const T d = 1 / (2*a);
			r1 = ( -b + sq ) * d;
			r2 = ( -b - sq ) * d;
			return true;//real roots
		}
		else
		{
			return false;//complex roots
		}
	}

	// solve system of 3 quadratic equations (y=ax^2 + bx + c) for a, b and c with known variables x and y
	// return false if no solution exists
	template <class T>
	bool solveQuadratic3(T x0, T x1, T x2, T y0, T y1, T y2, T& a, T& b, T& c)
	{
		TMatrix3<T> m;
		m(0, 0) = sqr(x0);
		m(0, 1) = sqr(x1);
		m(0, 2) = sqr(x2);
		m(1, 0) = x0;
		m(1, 1) = x1;
		m(1, 2) = x2;
		m(2, 0) = 1;
		m(2, 1) = 1;
		m(2, 2) = 1;

		bool result = m.invert();
		assert(result);
		if (!result)
			return false;

		TPoint3<T> coefficients = TPoint3<T>(y0, y1, y2) * m;
		a = coefficients[0];
		b = coefficients[1];
		c = coefficients[2];

		return true;
	}

	// return the velocity needed to propel a projectile from one point to another while under the influence of gravity
	// assumes z is up
	// returns Point3::MAX if no solution

	template <class T>
	TPoint3<T> projectileVelocityGivenVelX(const TPoint3<T>& p0, const TPoint3<T>& p1, T gravity, T initialVelX)
	{
		return projectileVelocity(p0, p1, gravity, initialVelX);
	}

	template <class T>
	TPoint3<T> projectileVelocityGivenVelZ(const TPoint3<T>& p0, const TPoint3<T>& p1, T gravity, T initialVelZ)
	{
		return projectileVelocity(p0, p1, gravity, FLT_MAX, initialVelZ);
	}

	template <class T>
	TPoint3<T> projectileVelocityGivenTime(const TPoint3<T>& p0, const TPoint3<T>& p1, T gravity, T time)
	{
		return projectileVelocity(p0, p1, gravity, FLT_MAX, FLT_MAX, time);
	}

	// either initial x vel, z vel or time must be specified. set unused knowns to FLT_MAX
	template <class T>
	TPoint3<T> projectileVelocity(const TPoint3<T>& p0, const TPoint3<T>& p1, T gravity, T initialVelX, T initialVelZ = FLT_MAX, T time = FLT_MAX)
	{
		TPoint2<T> vel;
		TPoint3<T> p0Top1 = (TPoint3<T>(p1.x, p1.y, 0) - TPoint3<T>(p0.x, p0.y, 0));
		float p0Top1Length = p0Top1.length();
		float t;

		p0Top1 /= p0Top1Length;

		if (initialVelX != FLT_MAX)
		{
			vel.x = initialVelX;
			t = p0Top1Length / vel.x;

			if (t == 0)
				return TPoint3<T>::MAX;

			vel.y = (abs(p1.z - p0.z) - 0.5f * sqr(t) * gravity) / t;
		}
		else if (initialVelZ != FLT_MAX)
		{
			vel.y = initialVelZ;

			float t0, t1;

			if (!solveQuadratic(-0.5f * gravity, vel.y, p0.z - p1.z, t0, t1))
				return TPoint3<T>::MAX;

			// tbd: is it necessary to check all these cases?
			if (t0 <= 0)
			{
				if (t1 <= 0)
					return TPoint3<T>::MAX;
				else
					t = t1;
			}
			else if (t1 <= 0)
			{
				if (t0 <= 0)
					return TPoint3<T>::MAX;
				else
					t = t0;
			}

			vel.x = p0Top1Length / t;
		}
		else if (time != FLT_MAX)
		{
			t = time;
			vel.x = p0Top1Length / t;
			vel.y = (abs(p1.z - p0.z) - 0.5f * sqr(t) * gravity) / t;
		}

		return TPoint3<T>(vel.x * p0Top1) + TPoint3<T>(0, 0, vel.y);
	}
}
