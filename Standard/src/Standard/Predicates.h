#include "Standard/api.h"

namespace Predicates
{
	template <class T>
	class Distance
	{
	public:
		Distance(T _reference = 0) :
		  reference(_reference)
		{}

		bool operator()(T first, T second)
		{
			return abs(reference - first) < abs(reference - second);
		}

		int reference;
	};
};