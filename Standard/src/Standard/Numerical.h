// ---------------------------------------------------------------------------------------------------------
// 
// Numerical Methods
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

template <class T>
struct IntegrateFunc
{
	typedef T ReturnType;

	T operator()(int t)
	{
		return T();
	}
}

template <class T>
T::ReturnType EulerIntegrate(const T& func, int steps)
{
	float delta = 1.0f / steps;
	float t = 0;
	T::ReturnType prev = func(t);
	T::ReturnType next;
	float length = 0;

	t += delta;
	for (int i = 1; i < steps; i++)
	{
		next = func(t);
		length += (nextPoint - prevPoint).length();

		prevPoint = nextPoint;
		t += delta;
	}

	return length;
}
