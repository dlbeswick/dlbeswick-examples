#pragma once

class IVisualisable
{
public:
	virtual int visBufferSize() const = 0; // in samples
	virtual void visGet(int position, int samples, void const*& data0, int& samples0, void const*& data1, int& samples1) const = 0;
	virtual void visGotten(void const* data0, int samples0, void const* data1, int samples1) const = 0;
	virtual int visPosition() const = 0; // in samples
	virtual int visBytesPerSample() const = 0;
	virtual int visSampleDistance(int pos0, int pos1) const = 0;

	// ruby hack, multiple inheritance not supported, this helps swig type check problems in client classes
	IVisualisable* visualisable() { return this; }
};
