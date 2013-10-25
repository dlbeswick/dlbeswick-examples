// ------------------------------------------------------------------------------------------------
//
// TGA
// 32 bit TGAs are stored as ARGB
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "TGA.h"
#include "Exception.h"
#include "Log.h"
#include "Path.h"

using namespace DMath;

// TGA
TGA::TGA()
{
	zero(m_header);
}

TGA::TGA(const Path& p)
{
	zero(m_header);
	ibinstream stream(p.open(std::ios::binary));
	tga_read(*p, stream);
}

TGA::TGA(const std::string& description, ibinstream& stream)
{
	zero(m_header);
	tga_read(description, stream);
}

void TGA::tga_read(const std::string& description, ibinstream& s)
{
	zero(m_header);
	if (m_data)
		delete m_data;
	if (m_palette)
	{
		delete m_palette;
		m_palette = 0;
	}

	// read header
	s.s().read((char*)&m_header, sizeof(m_header));

	if (!s.s())
		throwf("Bad TGA (header read error)");

	switch (m_header.encoding)
	{
	case 1:
		// 8 bit uncompressed
		m_format = BPP8;
		s.s().seekg(sizeof(m_header) + m_header.identLen);
		readPalette(s);
		genericRead(description, s, false);
		break;

	case 2:
		// Truecolor uncompressed
		if (m_header.depth == 24)
		{
			m_format = RGB24;
			genericRead(description, s, false);
			break;
		}
		else
		{
			m_format = ARGB32;
			genericRead(description, s, false);
			break;
		}

	case 9:
		// 8 bit RLE
		m_format = BPP8;
		readPalette(s);
		genericRead(description, s, true);
		break;

	case 10:
		// Truecolor RLE
		if (m_header.depth == 24)
		{
			m_format = RGB24;
			genericRead(description, s, true);
			break;
		}
		else
		{
			m_format = ARGB32;
			genericRead(description, s, true);
			break;
		}

	default:
		throwf("Sorry, only 8 or 32 bit TGAs are supported");
	}

	// flip image if needed
	if (!(m_header.desc & 0x20))
	{
		uint lineSize = m_header.width * bytesPerPixel();
		uchar* const newData = new uchar[lineSize * m_header.height];
		uchar* srcPos = 0;
		uchar* destPos = newData;

		switch (m_header.desc & 0x20)
		{
		case 0x00: // bottom left
			srcPos = m_data + calcOffset(Point2i(0, m_header.height - 1));
			do
			{
				memcpy(destPos, srcPos, lineSize);
				destPos += lineSize;
				srcPos -= lineSize;
			}
			while (srcPos >= m_data);
			break;
		case 0x10: // bottom right
			srcPos = m_data + calcOffset(Point2i(m_header.width - 1, m_header.height - 1));
			do
			{
				for (uint i = 0; i < lineSize; i += bytesPerPixel())
				{
					memcpy(destPos, srcPos, bytesPerPixel());
					srcPos -= bytesPerPixel();
					destPos += bytesPerPixel();
				}
			}
			while (srcPos >= m_data);
			break;
		case 0x30: // top right
			{
				srcPos = m_data + calcOffset(Point2i(m_header.width - 1, 0));
				uchar* srcEnd = m_data + m_header.width * m_header.height * bytesPerPixel();
				do
				{
					for (uint i = 0; i < lineSize; i += bytesPerPixel())
					{
						memcpy(destPos, srcPos, bytesPerPixel());
						srcPos -= bytesPerPixel();
						destPos += bytesPerPixel();
					}
					srcPos += lineSize * 2;
				}
				while (srcPos != srcEnd);
			}
			break;
		};

		delete m_data;
		m_data = newData;
	}
}

void TGA::read(const std::string& description, ibinstream& s)
{
	tga_read(description, s);
}

void TGA::write(obinstream& s) const
{
	// write header
	s.s().write((char*)&m_header, sizeof(m_header));

	genericWriteRLE(s);
}

void TGA::create(int width, int height, COMMONFORMAT format)
{
	Image::create(width, height, format);

	zero(m_header);

	switch (format)
	{
	case RGB24:
		{
			m_header.depth = 24;
			m_header.encoding = 10;
			break;
		}
	case ARGB32:
		{
			m_header.depth = 32;
			m_header.encoding = 10;
			m_header.desc |= 0x08;
			break;
		}
	default:
		throw(ExceptionString("Image create - unsupported format."));
	};

	// set image to be loaded from top-left
	m_header.desc |= 0x20;
	m_header.width = width;
	m_header.height = height;
}

void TGA::colourKey(const RGBA& col, const RGBA& replacement)
{
	if (m_header.depth != 32)
		throwf("Attempt to colour-key an image, but image bit depth is less than 32 bits.");

	uint* data = (uint*)m_data;
	uint keyCol = col;
	for (int i = 0; i < m_header.width * m_header.height; ++i)
	{
		if (*data == keyCol)
			*data = replacement;

		++data;
	}
}
