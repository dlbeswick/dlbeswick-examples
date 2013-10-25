#include "BufferModifier.h"

namespace Effect
{
	class Sine : public Base<Sine>
	{
	public:
		Sine() :
			c(0),
			frequency(100)
		{
		}

		template <class T>
		bool modify(T*& buf, int& samples, int stride)
		{
			T* b = buf;

			for (int i = 0; i < samples; ++i)
			{
				float f = frequency / ((((int)c % 44100) / 22050) + 1);

				*b++ = combineDryWet(*b, (T)(sin(2.0f * 3.14f * (c / 44100.0f) * f) * 32767.0f));
				*b++ = combineDryWet(*b, (T)(sin(2.0f * 3.14f * (c / 44100.0f) * f) * 32767.0f));
				++c;
			}
			return true;
		}

		float frequency;
		float c;
	};
}
