#include "BufferModifier.h"
#include "BufferFormat.h"
#include "Gain.h"
#include "Standard/Thread/SafeCopyAccessor.h"

#if IS_MSVC
#pragma warning(disable:4244)
#endif

namespace Effect
{
	class GainEffect : public Base<GainEffect>
	{
	public:
		GainEffect() :
			_gain(0),
			_scale(1)
		{
		}

		template <class T>
		bool modify(T*& buf, int samples, int stride)
		{
			T* b = buf;

			float scale = _scale;

			for (int i = 0; i < samples; ++i)
			{
				*b++ *= (T)scale;
				*b++ *= (T)scale;
			}

			return true;
		}

		void setGain(float gain_db)
		{
			_gain = Gain(gain_db);
			_scale = _gain.scale();
		}

		void setGain(const ::Gain&  g)
		{
			_gain = g;
			_scale = _gain.scale();
		}

		::Gain gain() const
		{
			return _gain;
		}

	protected:
		StandardThread::SafeCopyAccessor<float> _scale;
		::Gain _gain;
	};
}
