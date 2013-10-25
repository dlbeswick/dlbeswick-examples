#include "BufferFormat.h"
#include "SoundVisualisation.h"

class SoundVisualisationProgressive : public SoundVisualisation
{
public:
	SoundVisualisationProgressive(const IVisualisable& target, int samples, int width, int height);

	void draw();

protected:
	void doDraw(int drawStart, int drawEnd);

	int m_prevSampleEnd;
	int m_drawCursor;

	typedef std::vector<float> Samples;
	Samples m_sampleData;
};
