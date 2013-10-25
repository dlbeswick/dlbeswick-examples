// ------------------------------------------------------------------------------------------------
//
// Mapping
// Helps in defining mappings from one range of values to another
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"

// Functions that map one range to another
namespace Mapping
{
	template <class T>
	inline T linear(T value, T sourceMin, T sourceMax, T destMin, T destMax)
	{
		assert(sourceMax - sourceMin != 0);
		return destMin + ((value - sourceMin) / (sourceMax - sourceMin)) * (destMax - destMin);
	}

	template <class T>
	inline T exp(T value, T sourceMin, T sourceMax, T destMin, T destMax)
	{
		assert(sourceMax - sourceMin != 0);
		float normalized = (value - sourceMin) / (sourceMax - sourceMin);
		return destMin + (normalized * normalized) * (destMax - destMin);
	}

	template <class T>
	inline T power(T value, T sourceMin, T sourceMax, T destMin, T destMax, T power)
	{
		assert(sourceMax - sourceMin != 0);
		float normalized = (value - sourceMin) / (sourceMax - sourceMin);
		return destMin + pow(normalized, power) * (destMax - destMin);
	}
}
