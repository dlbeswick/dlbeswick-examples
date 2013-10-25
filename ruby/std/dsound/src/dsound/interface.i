%{
#undef read
#undef write
#undef close
%}

%runtime %{
#include "dsound/common.h"
#include "dsound/EffectFilter.h"
#include "dsound/EffectFrequency.h"
#include "dsound/EffectGain.h"
#include "dsound/EffectHold.h"
#include "dsound/EffectMixGeneratorOutputs.h"
#include "dsound/EffectSine.h"
#include "dsound/EffectEncode.h"
#include "dsound/EffectLimiter.h"
#include "dsound/EffectResample.h"
#include "dsound/ModifierNode.h"
#include "dsound/MP3Decoder.h"
#include "dsound/SoundBuffer.h"
#include "dsound/SoundBufferMedia.h"
#include "dsound/SoundConsumer.h"
#include "dsound/SoundDevice.h"
#include "dsound/SoundGenerator.h"
#include "dsound/SoundGeneratorOutput.h"
#include "dsound/SoundLibrary.h"
#include "dsound/SoundVisualisation.h"
#include "dsound/SoundVisualisationProgressive.h"
#if !IS_OSX
  #include "Standard/Wavelet.h"
#endif
%}
typedef unsigned int uint;

%module dsound

%include std_vector.i
%include std_string.i

%template(SoundGeneratorVector) std::vector<SoundGeneratorOutput*>;
%template(VectorFloat) std::vector<float>;

%exception {
 try {
   $action
 }
 catch (const Exception& e) {
   rb_raise(rb_eStandardError, e.what());
 }
}

//#define DSOUND_SWIG_CAPABILITIES 1
//#include "dsound/capabilities.h"

class IVisualisable
{
protected:
	IVisualisable();
};

class Gain
{
public:
    Gain(float db, float dbMin=-144);
    
    bool operator == (const Gain& rhs) const;

    Gain operator + (const Gain& rhs) const;
    
    float db() const;
    
    float scale() const;

    void setDb(float db);
};

namespace StandardThread
{
	template <class T>
	class SafeCopyAccessor
	{
	public:
		SafeCopyAccessor(T value);
		
		T get();
		void set(const T& value);
	};
};

%template(SafeCopyAccessorFloat) StandardThread::SafeCopyAccessor<float>;

class SoundConsumer
{
public:
	SoundConsumer();
	~SoundConsumer();

	void addInput(SoundGeneratorOutput& output);
	std::vector<SoundGeneratorOutput*> inputs() const;
	void removeInput(SoundGeneratorOutput& output);
	void clearInputs();
};

class SoundLibrary
{
public:
	SoundLibrary(const std::string& client_name) {}
	
	virtual void close() = 0;
	SoundDevice& defaultDevice() const;
	std::vector<SoundDevice*> devices() const;
	
	virtual float desiredLatency() const = 0;
	virtual void setDesiredLatency(float latency) = 0;
	
	virtual void open() = 0;
	virtual void refreshDevices() = 0;
};

class SoundDevice : public SoundConsumer
{
public:
	SoundDevice(float latency = 0.05f) {};

	virtual std::string description() const = 0;
	
  virtual bool opened() const = 0;
	virtual bool playing() const = 0;
	virtual void setPlayCursor(int samples) = 0;

	virtual int playCursor() const = 0;
	virtual int writeCursor() const = 0;

	virtual float samplesPerSec() const = 0;

	virtual void syncRequestAdd(
		SoundGeneratorOutput& source, 
		SoundGeneratorOutput& dest, 
		SoundDevice& syncee
	);
};

class SoundModifier // : public SoundConsumer, public SoundGenerator
{
public:
	SoundGeneratorOutput* requestOutput();

	virtual void setEnabled(bool b);
	virtual bool enabled() const;
	
	std::vector<SoundGeneratorOutput*> outputs() const;

	void addInput(SoundGeneratorOutput& output);
	std::vector<SoundGeneratorOutput*> inputs() const;
};

class BufferModifierHost
{
public:
	virtual void addModifier(BufferModifier& modifier, int priority = 0);
	virtual void removeModifier(BufferModifier& modifier);

	void duplicateModifiers(BufferModifierHost& dest);

    typedef std::vector<BufferModifier*> SortedModifiers;
    SortedModifiers modifiers() const;
    
protected:
	BufferModifierHost() {}
};

class SoundGenerator
{
public:
	SoundGeneratorOutput* requestOutput();

	virtual void setEnabled(bool b);
	virtual bool enabled() const;
	
	std::vector<SoundGeneratorOutput*> outputs() const;
	
protected:
	SoundGenerator();
};

class SoundGeneratorOutput
{
public:
	SoundGeneratorOutput(SoundGenerator& generator);

	void invalidate();

	bool valid() const;
	void validate();

	SoundGenerator& m_generator;

	void safeSetReadCursor(int pos);

	int safeReadCursor() const;

	void safeSetRemainingSamples(int samples);

	int safeRemainingSamples() const;
};

class BufferModifier
{
public:
	BufferModifier();

	virtual ~BufferModifier();

	virtual void modify(short* buf, int samples, int stride);
	virtual void modify(float* buf, int samples, int stride);

	virtual int priority() const;

	float dryToWetMultiplier() const;

	virtual void setDryToWet(float v);

	virtual void setEnabled(bool b);
	virtual bool enabled() const;
};

class ModifierNode : public SoundModifier // , public BufferModifierHost, public IVisualisable
{
public:
	ModifierNode(float bufferSeconds = 1.0f);
	~ModifierNode();

	virtual void addModifier(BufferModifier& modifier, int priority = 0);
	virtual void removeModifier(BufferModifier& modifier);

	void addInput(SoundGeneratorOutput& output);

	IVisualisable* visualisable();

    typedef std::vector<BufferModifier*> SortedModifiers;
    SortedModifiers modifiers() const;
};

class SoundBuffer : public SoundGenerator
{
public:
	SoundBuffer(SoundLibrary& library, float bufferLengthSeconds = 0.4f, int buffers = 2);

	virtual ~SoundBuffer();

	virtual void restart();
	
	virtual void initBuffer();

	virtual double progress() const = 0;

	virtual void setProgress(double ms) = 0;

	virtual void stopThread();
};

class SoundBufferMedia : public SoundBuffer
{
public:
	SoundBufferMedia(SoundLibrary& library, float seconds = 1.0f);

	virtual void open(const char* filePath);
	
	virtual double progress() const;
	virtual void setProgress(double ms);
	virtual double lengthMS() const;
	virtual void waitForLengthCalculation() const ;
	virtual bool opening() const;
	virtual bool opened() const;
	virtual int rate() const;
};

class FilterKernelFunction
{
public:
	virtual void operator() (float* buf, int size) = 0;
	virtual int length() const = 0;
	virtual bool frequencyDomain() const = 0;
};

class BlackmanSincLowpass : public FilterKernelFunction
{
public:
	BlackmanSincLowpass(double _cutoff, double _bandwidth);
	
	virtual void operator() (float* buf, int size);
	virtual int length() const;
	virtual bool frequencyDomain() const;

	double bandwidth;
	double cutoff;
};

class BlackmanSincHighpass : public BlackmanSincLowpass
{
public:
	BlackmanSincHighpass(double _cutoff, double _bandwidth);

	virtual void operator() (float* buf, int size);
};

#if 0
class FilterKernel
{
public:
	FilterKernel(FilterKernelFunction& _function);

	void eval(bool lazy = true);
};

class FrequencyResponseFFT : public FilterKernelFunction
{
public:
	FrequencyResponseFFT(int size);

	%extend {
		VALUE set(VALUE ary) 
		{
			int len = NUM2INT(rb_funcall(ary, rb_intern("length"), 0));
			float* buf = new float[len];
			
			for (int i = 0; i < len; ++i)
			{
				buf[i] = NUM2DBL(rb_ary_entry(ary, i));
			}
			
			self->set(buf);
			
			delete[] buf;
			
			return Qnil;
		}
	}	

	virtual int length() const;
	virtual void operator() (float* buf, int size) ;
	virtual bool frequencyDomain() const;
};

class FrequencyResponse : public FrequencyResponseFFT
{
	FrequencyResponse(int size);
};

class FilterKernelFFT : public FilterKernel
{
public:
	FilterKernelFFT(FilterKernelFunction& _function);
};

class Filter : public BufferModifier
{
public:
	Filter(FilterKernel& kernel);
	const FilterKernel& kernel;
};

#endif

namespace Effect
{
	class FilterIIR : public BufferModifier
	{
	public:
		void setCoefficients(const std::vector<float>& coefficientsA, const std::vector<float>& coefficientsB);
	};

	class FilterOnePole : public FilterIIR
	{
	public:
		float cutoff;

	protected:
		virtual void setCutoff(float freq) = 0;
	};

	class FilterLowpass : public FilterOnePole
	{
	protected:
		virtual void setCutoff(float freq);
	};

	class FilterHighpass : public FilterOnePole
	{
	protected:
		virtual void setCutoff(float freq);
	};

	class Sine : public BufferModifier
	{
	public:
		float frequency;
	};

	class GainEffect : public BufferModifier
	{
	public:
		void setGain(float gain_db);
		void setGain(const ::Gain&  g);
		Gain gain() const;
	};

	class Frequency : public BufferModifier
	{
	public:
		StandardThread::SafeCopyAccessor<float> frequencyScale;
	};

	class MixGeneratorOutputs : public BufferModifier
	{
	public:
		MixGeneratorOutputs(float seconds = 0.5f);

		void add(SoundGeneratorOutput& g);
	};
	
#ifndef NOMP3
	class EncodeMP3 : public BufferModifier
	{
	public:
		EncodeMP3(SoundLibrary& library, const char* path="", bool safe=true);
		void setPath(const char* _path);
		void destroy();
	};
#endif

#if WITH_FLAC
	class EncodeFlac : public BufferModifier
	{
	public:
		EncodeFlac(SoundLibrary& library, const char* path="", bool safe=true);
		void setPath(const char* _path);
		void destroy();
	};
#endif
	
	class Limiter : public BufferModifier
	{
	public:
		Limiter(const Gain& dbThreshold=Gain(-0.2f), float attackSeconds=1.0f, float releaseSeconds=1.0f);

		void setThreshold(const Gain& dbThreshold);
		void setAttack(float time);
		void setRelease(float time);
	};

	class Hold : public BufferModifier
	{
	public:
		Hold(float maxSeconds=0.5f);

		void hold();

		void release();

		float maxHoldSeconds() const;
		float holdSeconds() const;
		void setHoldSeconds(float seconds);
	};
};

class EffectResample : public BufferModifier
{
public:
  void setInputRate(int rate);
  void setOutputRate(int rate);
};

class SoundVisualisation
{
public:
	virtual ~SoundVisualisation();

	virtual void draw();

	virtual void setSize(int width, int height);
	
	virtual int height() const;
	virtual int width() const;
	
	%extend {
		VALUE ruby_buffer(VALUE str) {
			int elements = self->width() * self->height();
			int* buf = self->buffer();
			
			if (str == Qnil)
				str = rb_str_new((char*)buf, elements * sizeof(int));
			else
				memcpy(StringValuePtr(str), buf, elements * sizeof(int));
			
			return str;
		}
	}	
protected:
	SoundVisualisation();
};

class SoundVisualisationProgressive : public SoundVisualisation
{
public:
	SoundVisualisationProgressive(const IVisualisable& target, int samples, int width, int height);

	void draw();
};

#if !IS_OSX
class WaveletDiscreteFloat
{
public:
  typedef float Element;
  
  uint output_vectors_for(uint input_size);
  uint output_size_for(uint input_size);
  uint output_size_bytes_for(uint input_size);

	void calc(DSOUND_NumericPtr input_address, uint input_size, DSOUND_NumericPtr output_address, uint output_size, DSOUND_NumericPtr work_address, uint work_size);
  void calc(const std::string& input_path, const std::string& output_path, const std::string& work_path);

	void invert(long int input_address, uint input_size, long int output_address, uint output_size, long int work_address, uint work_size);
  void invert(const std::string& input_path, const std::string& output_path, const std::string& work_path);

protected:
  WaveletDiscreteFloat();
};

class WaveletLiftingHaar : public WaveletDiscreteFloat
{
public:
  WaveletLiftingHaar();

	void calc(unsigned long long input_address, uint input_size, unsigned long long output_address, uint output_size, unsigned long long work_address, uint work_size);
  void calc(const std::string& input_path, const std::string& output_path, const std::string& work_path);

	void invert(unsigned long long input_address, uint input_size, unsigned long long output_address, uint output_size, unsigned long long work_address, uint work_size);
  void invert(const std::string& input_path, const std::string& output_path, const std::string& work_path);
};
#endif
