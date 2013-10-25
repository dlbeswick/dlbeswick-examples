#include "dsound/pch.h"
#include "SoundVisualisationProgressive.h"
#include "BufferFormat.h"
#include "IVisualisable.h"
#include "SoundBuffer.h"
//#include "Standard/Predicates.h"

SoundVisualisationProgressive::SoundVisualisationProgressive(const IVisualisable& target, int samples, int width, int height) :
	SoundVisualisation(target, samples, width, height),
	m_prevSampleEnd(0),
	m_drawCursor(0)
{
}

void SoundVisualisationProgressive::draw()
{
	SoundVisualisation::draw();

	int sampleBegin = m_prevSampleEnd;
	int sampleEnd = m_target.visPosition();
	assert(sampleEnd < m_target.visBufferSize());
	int samplesPerColumn = (int)((float)m_samples / m_width);

	if (m_target.visSampleDistance(sampleBegin, sampleEnd) < samplesPerColumn)
		return;

	char const* data0;
	int samples0;
	char const* data1;
	int samples1;
	m_target.visGet(sampleBegin, m_target.visSampleDistance(sampleBegin, sampleEnd), (void const*&)data0, samples0, (void const*&)data1, samples1);

	m_prevSampleEnd = sampleEnd;

	int bytesPerSample = m_target.visBytesPerSample();

	for (int i = 0; i < samples0 * 2; ++i)
	{
		m_sampleData.push_back(BufferFormat::anyToFloat(bytesPerSample, (void*)data0));
		data0 += bytesPerSample / 2;
	}

	for (int i = 0; i < samples1 * 2; ++i)
	{
		m_sampleData.push_back(BufferFormat::anyToFloat(bytesPerSample, (void*)data1));
		data1 += bytesPerSample / 2;
	}

	m_target.visGotten(data0, samples0, data1, samples1);

	int columnsToDraw = std::min<int>((m_sampleData.size() / 2) / samplesPerColumn, m_width);

	if (columnsToDraw == 0)
		return;

	int drawEnd0 = m_drawCursor + columnsToDraw;
	int drawEnd1;
	if (drawEnd0 > m_width)
	{
		drawEnd1 = drawEnd0 - m_width;
		drawEnd0 = m_width;

		doDraw(m_drawCursor, drawEnd0);
		m_drawCursor = 0;
		doDraw(m_drawCursor, drawEnd1);
	}
	else
	{
		doDraw(m_drawCursor, drawEnd0);
	}
}

void SoundVisualisationProgressive::doDraw(int drawStart, int drawEnd)
{
	assert(drawEnd - m_drawCursor > 0);

	int* draw = m_buffer;

	clear(m_width, m_height, draw, m_drawCursor, 0, drawEnd, m_height-1);

	Samples::iterator sampleIt = m_sampleData.begin();

	int samplesPerColumn = (int)((float)m_samples / m_width);
	int columnsToDraw = drawEnd - drawStart;

	for (int i = 0; i < columnsToDraw; ++i)
	{
		assert(m_drawCursor >= 0 && m_drawCursor < m_width);

		//float e = (float)*std::max_element(sampleIt, sampleIt + samplesPerColumn, Predicates::Distance<float>());
		float e = 0;
		sampleIt += samplesPerColumn * 2;

		clamp(e, -BufferFormat::LocalFormat::max(), BufferFormat::LocalFormat::max());
		e = Mapping::linear<float>(abs(e), 0, BufferFormat::LocalFormat::max(), 0, m_height);

		int startY = (int)(m_height / 2 - e / 2);
		int endY = (int)(m_height / 2 + e / 2);

		int* column = draw + m_drawCursor + startY * m_width;

		for (int y = startY; y < endY; ++y)
		{
			assert(y >= 0 && y < m_height);
			*column = 0x0000FF00;
			column += m_width;
		}

		m_drawCursor = (m_drawCursor + 1) % (m_width-1);
	}

	m_sampleData.erase(m_sampleData.begin(), sampleIt);
}
