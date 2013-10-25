#include "dsound/pch.h"
#include "SoundVisualisation.h"

SoundVisualisation::SoundVisualisation(const IVisualisable& target, int samples, int width, int height) :
	m_samples(samples),
	m_target(target),
	m_lastBuffer(0),
	m_buffer(0)
{
	setSize(width, height);
}

SoundVisualisation::~SoundVisualisation()
{
	delete[] m_buffer;
}

void SoundVisualisation::draw()
{
	if (m_lastBuffer != m_buffer)
		clear(m_width, m_height, m_buffer, 0, 0, m_width-1, m_height-1);

	m_lastBuffer = m_buffer;
}

void SoundVisualisation::clear(int width, int height, int* buffer, int x0, int y0, int x1, int y1)
{
	assert(y1 - y0 > 0);
	assert(x1 - x0 > 0);

	for (int i = y0; i < y1; ++i)
	{
		memset(buffer + width * i + x0, 0, sizeof(int) * (x1 - x0));
	}
}

void SoundVisualisation::setSize(int width, int height)
{
	m_width = width;
	m_height = height;
	delete[] m_buffer;
	m_buffer = new int[m_width * m_width * 4];
}
