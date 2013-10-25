#ifndef STANDARD_BUFFERMODIFIER_H
#define STANDARD_BUFFERMODIFIER_H

class BufferModifierHost;

class BufferModifier
{
public:
	BufferModifier() :
	  m_dryToWet(1.0f),
	  m_priority(0),
	  m_enabled(true),
	  m_host(0)
	  {}

	virtual ~BufferModifier();

	bool operator < (const BufferModifier& rhs) const
	{
		return priority() < rhs.priority();
	}

	virtual bool modify(short*& buf, int& samples, int stride) { assert(0); return false; }
	virtual bool modify(float*& buf, int& samples, int stride) { assert(0); return false; }

	virtual int priority() const
	{
		Critical(_critical);
		return m_priority;
	}

	__forceinline float dryToWetMultiplier() const
	{
		Critical(_critical);
		return m_dryToWet;
	}

	virtual void setDryToWet(float v)
	{
		Critical(_critical);
		m_dryToWet = v;
	}

	template <class T>
	__forceinline T combineDryWet(T dry, T wet)
	{
		Critical(_critical);
		return (T)(dry * (1.0f - m_dryToWet) + wet * m_dryToWet);
	}

	virtual void setEnabled(bool b);
	virtual bool enabled() const 
	{ 
		Critical(_critical);
		return m_enabled; 
	}

	virtual void setHost(BufferModifierHost* host) 
	{ 
		Critical(_critical);
		m_host = host; 
	}

	int bitsPerSample() const { return 32; }
	int channels() const { return 2; }
	inline float samplesPerSec() const { return 44100.0; }

	static int minimumSamples() { return 128; }

protected:
	friend class BufferModifierHost;

	BufferModifierHost* m_host;
	float m_dryToWet;
	int m_priority;
	bool m_enabled;
	CriticalSection _critical;
};

class BufferModifierClear : public BufferModifier
{
public:
	virtual bool modify(short*& buf, int& samples, int stride);
	virtual bool modify(float*& buf, int& samples, int stride);
};

#endif
