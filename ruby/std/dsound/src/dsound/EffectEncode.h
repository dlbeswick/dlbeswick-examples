#ifndef DSOUND_EFFECTENCODE_H
#define DSOUND_EFFECTENCODE_H

#ifndef NOMP3

#include "dsound/BufferModifier.h"
#if IS_WINDOWS && HAVE_BLADEENC
#include "dsound/MP3EncoderBladeEnc.h"
#else
#include "dsound/MP3EncoderLame.h"
#endif

#if WITH_FLAC
#include "dsound/TranscoderFlac.h"
#endif

#include "Standard/Log.h"

namespace Effect
{
	class Encode : public Base<Encode>
	{
	public:
		Encode(SoundLibrary& library, const char* _path="", bool _safe=true) :
			encoder(0),
			path(_path),
			safe(_safe),
			executed(false),
			_library(library)
		{
		}

		virtual ~Encode()
		{
			if (encoder)
			{
				encoder->destroy();
				delete encoder;
			}
		}

		void setPath(const char* _path)
		{
			Critical(_critical);

			if (path != _path)
			{
				path = _path;
				if (executed) // tbd: not sure this is ever done
				{
					if (enabled())
						initializeEncoderModule();
				}
			}
		}

		virtual void destroy()
		{
			Critical(_critical);
			encoder->destroy();
		}

		virtual void setEnabled(bool b)
		{
			Critical(_critical);

			if (!b && encoder)
			{
				encoder->flush();
			}
			else if (b && !encoder)
			{
				if (!safe)
				{
					initializeEncoderModule();
				}
				else
				{
					try
					{
						initializeEncoderModule();
					}
					catch (Exception& e)
					{
						derr << "EffectEncode: Exception writing file (" << e.what() << ")" << derr.endl;
						b = false;
					}
				}
			}

			Base<Encode>::setEnabled(b);
		}

		template <class T>
		bool modify(T*& buf, int& samples, int stride)
		{
			Critical(_critical);
			executed = true;

			if (!safe)
			{
				encoder->write((void*)buf, samples * BufferFormat::Local.bytesPerSample());
			}
			else
			{
				try
				{
					encoder->write((void*)buf, samples * BufferFormat::Local.bytesPerSample());
				}
				catch (Exception& e)
				{
					derr << "EffectEncode: Exception writing file (" << e.what() << ")" << derr.endl;
					setEnabled(false);
				}
			}

			return false;
		}

	protected:
		void initializeEncoderModule()
		{
			Critical(_critical);

			if (encoder)
			{
				encoder->destroy();
				delete encoder;
			}

			encoder = constructTranscoder();

			dlog << "EffectEncode: Writing to " << path << derr.endl;
		}

		virtual Transcoder* constructTranscoder() = 0;

		std::string path;
		Transcoder* encoder;
		bool safe;
		bool executed;
		SoundLibrary& _library;
	};

	class EncodeMP3 : public Encode
	{
	public:
		EncodeMP3(SoundLibrary& library, const char* _path="", bool _safe=true) :
			Encode(library, _path, _safe)
		{}

		virtual Transcoder* constructTranscoder()
		{
#if IS_WINDOWS && HAVE_BLADEENC
			return new MP3EncoderBladeEnc(_library, path.c_str());
#else
			return new MP3EncoderLame(_library, samplesPerSec(), channels(), path.c_str());
#endif
		}
	};

#if WITH_FLAC
	class EncodeFlac : public Encode
	{
	public:
		EncodeFlac(SoundLibrary& library, const char* _path="", bool _safe=true) :
			Encode(library, _path, _safe)
		{}

		virtual Transcoder* constructTranscoder()
		{
			return new TranscoderFlac(_library, samplesPerSec(), channels(), path.c_str());
		}
	};
#endif
}

#endif

#endif
