// ------------------------------------------------------------------------------------------------
//
// Interpolation functions
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_INTERPOLATION_H
#define STANDARD_INTERPOLATION_H

#if IS_MSVC
#pragma warning(disable:4244) // loss of data, can occur when using with type int and scalar float
#endif

#include "Standard/api.h"
#include "Algebra.h"
#include "Help.h"
#include "Mapping.h"

namespace Interpolation
{
	using namespace DMath;

	template <class T, class Scalar>
	class Adapter
	{
	public:
		typedef T (*FuncType)(const T&, const T&, Scalar);

		Adapter(FuncType func) :
			m_func(func)
		{}

		T operator () (const T& a, const T& b, Scalar c) const
		{
			return m_func(a, b, c);
		}

	protected:
		FuncType m_func;
	};

	template <class T, class Scalar>
	inline T linear(const T& start, const T& end, Scalar u)
	{
		return start + (end - start) * u;
	}

	template <class T, class Scalar>
	inline T exp(const T& start, const T& end, Scalar u)
	{
		return start + (end - start) * (u * u);
	}

	template <class T, class Scalar>
	inline T power(const T& start, const T& end, float power, Scalar u)
	{
		return start + (end - start) * pow(u, power);
	}

	template <class T, class Scalar>
	inline TQuat<T> linear(const TQuat<T>& start, const TQuat<T>& end, Scalar u)
	{
		TQuat<T> result = start + (end - start) * u;
		result.normalise();
		return result;
	}

	// http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
	template <class T, class Scalar>
	inline T slerp(const T& start, const T& end, Scalar u)
	{
		Scalar dot = start.dot(end);

		const Scalar DOT_THRESHOLD = 0.9995f;
		if (dot > DOT_THRESHOLD)
		{
			// If the inputs are too close for comfort, linearly interpolate
			// and normalize the result.

			return linear(start, end, u);
		}

		clamp(dot, -1.0f, 1.0f);           // Robustness: Stay within domain of acos()
		Scalar theta_0 = acos(dot);  // theta_0 = angle between input vectors
		Scalar theta = theta_0*u;    // theta = angle between start and result

		T v2 = end - start * dot;
		v2.normalise();              // { start, v2 } is now an orthonormal basis

		return start*cos(theta) + v2*sin(theta);
	}

#if 0
	template <class T, class Scalar>
	class TCubicSpline2
	{
	public:
		TCubicSpline2(const TPoint2<T>& p0, const TPoint2<T>& p1, const TPoint2<T>& p2)
		{
			TMatrix<T, 3> a;
			TPoint3<T> k;
			TPoint3<T> b;

			_succeeded = a.invert();

			k = b * a;

			_a1 = k.x*(p1.x - p0.x) - (p1.y - p0.y);
			_b1 = -k.y*(p1.x - p0.x) + (p1.y - p1.y);
			_a2 = k.y*(p2.x - p1.x) - (p2.y - p1.y);
			_b2 = -k.z*(p2.x - p1.x) + (p2.y - p1.y);
		}

		T operator()(Scalar u) const
		{
			u *= 2;

			if (u < 1)
			{
				return (1 - u)*p0.x + u*p1.x + u*(1-u)*(_a1*(1-u) + _b1*u);
			}
			else
			{
				u -= 1;
				return (1 - u)*p1.x + u*p2.x + u*(1-u)*(_a2*(1-u) + _b2*u);
			}
		}

		bool succeeded() const { return _success; }

	private:
		bool _succeeded;
		TPoint2<T>::Element _a1;
		TPoint2<T>::Element _b1;
		TPoint2<T>::Element _a2;
		TPoint2<T>::Element _b2;
	};
#endif

	template <class T>
	class TParabola
	{
	public:
		TParabola(const TPoint3<T>& p0, const TPoint3<T>& p1, const TPoint3<T>& p2)
		{
			recalculate(p0, p1, p2);
		}

		void recalculate(const TPoint3<T>& p0, const TPoint3<T>& p1, const TPoint3<T>& p2)
		{
			// equation of parabola in cartesian space is y=ax^2 + bx + c
			// solve linear system of equations to find a, b and c given three points
			Algebra::solveQuadratic3(p0.x, p1.x, p2.x, p0.z, p1.z, p2.z, coefficients[0], coefficients[1], coefficients[2]);

			startY = p0.y;
			endY = p2.y;

			// parabola function domain ranges from start x value to end x value
			startT = p0.x;
			endT = p2.x;
		}

		// u is from 0 to 1
		TPoint3<T> operator ()(T u)
		{
			TPoint3<T> p;
			u = Mapping::linear<T>(u, 0.0f, 1.0f, startT, endT);
			p.x = u;
			p.y = Mapping::linear<T>(u, 0.0f, 1.0f, startY, endY);
			p.z = coefficients.x * sqr(u) + coefficients.y * u + coefficients.z;
			return p;
		}

		// derivative can be used to find the velocity at a given point in the function
		//
		//TPoint3<T> velocity(T u)
		//{
		//	y = 2ax + b; untested
		//	x = (y-b) / 2a;
		//}

		T startY;
		T endY;
		T startT;
		T endT;
		TPoint3<T> coefficients;
	};

	class Parabola : public TParabola<float> 
	{
	public:
		Parabola(const TPoint3<float>& p0, const TPoint3<float>& p1, const TPoint3<float>& p2) :
			TParabola<float>(p0, p1, p2) {}
	};
}

#endif
