#include "BufferFormat.h"
#include "BufferModifierHost.h"
#include "IVisualisable.h"
#include "SoundModifier.h"

// This node caches the result of its ancestor connected nodes, which is necessary when multiple leaf nodes (for instance,
// external sources with differing clocks) will be pulling data from the modifier tree.
// It also applies BufferModifiers such as effects to its input.
// It should be renamed to 'CacheNode'.
class ModifierNode : public SoundModifier, public BufferModifierHost, public IVisualisable
{
public:
	ModifierNode(float bufferSeconds = 1.0f);
	~ModifierNode();

	// IVisualisable
	virtual int visBufferSize() const; // in samples
	virtual int visBytesPerSample() const;
	virtual void visGet(int position, int samples, void const*& data0, int& samples0, void const*& data1, int& samples1) const;
	virtual void visGotten(void const* data0, int samples0, void const* data1, int samples1) const {}
	virtual int visPosition() const;
	virtual int visSampleDistance(int pos0, int pos1) const;

protected:
	virtual bool get(SoundGeneratorOutput& output, BufferFormat::LocalFormat::SampleElement* buf, int samples);

	int m_samplesRemaining;
	int m_writeCursor;
	int m_lastWritePos;
	BufferFormat::LocalFormat::SampleElement* m_work;
	CircularBuffer<BufferFormat::LocalFormat::SampleElement> m_buffer;
};
