#ifndef DSOUND_SOUNDVISUALISATION_H
#define DSOUND_SOUNDVISUALISATION_H

class SoundBuffer;
class IVisualisable;

class SoundVisualisation
{
public:
	virtual ~SoundVisualisation();

	virtual void draw();

	virtual void setSize(int width, int height);

	virtual int* buffer() const { return m_buffer; }

	virtual int height() const { return m_height; }
	virtual int width() const { return m_width; }

protected:
	SoundVisualisation(const IVisualisable& target, int samples, int width, int height);

	void clear(int width, int height, int* buffer, int x0, int y0, int x1, int y1);

	int m_width;
	int m_height;
	int* m_buffer;
	int* m_lastBuffer;
	const IVisualisable& m_target;
	int m_samples;
};

#endif
