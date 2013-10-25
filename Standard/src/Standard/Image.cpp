// ------------------------------------------------------------------------------------------------
//
// Image
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Image.h"
#include "binstream.h"
#include "FileMgr.h"
#include "Log.h"
#include "Math.h"

using namespace DMath;

STANDARD_API int Image::m_bytesPerPixel[Image::FORMAT_NUM] =
{
	1,// BPP8
	3,// RGB24
	4,// ARGB32
};

// pixel-specific functions
class PixelFunc24Bit
{
public:
	__forceinline void readRLE(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		std::istream& str = s.s();

		char p[3];
		str.read((char*)p, 3);
		if (!str)
		{
			derr << "Malformed 24 bit image " << description << "\n";
			return;
		}

		while (count--)
		{
			std::copy(p, p + 3, data);
			data += 3;
		}
	}

	__forceinline void readRaw(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		std::istream& str = s.s();

		str.read((char*)data, count * 3);
		if (!str)
		{
			derr << "Malformed 32 bit image " << description << "\n";
			return;
		}
	}

	__forceinline void writeRLE(uchar* data, obinstream& s)
	{
		s.s().write((char*)data, 3);
	}

	__forceinline bool pixelCompare(uchar* lhs, uchar* rhs)
	{
		return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
	}
};

class PixelFunc32Bit
{
public:
	__forceinline void readRLE(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		uint pixelVal;
		s.s().read((char*)&pixelVal, 4);
		if (!s.s())
		{
			derr << "Malformed 32 bit image " << description << "\n";
			return;
		}

		dword* ddata = (dword*)data;

		for (int i = 0; i < count; ++i)
			*ddata++ = pixelVal;
	}

	__forceinline void readRaw(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		s.s().read((char*)data, count * 4);
		if (!s.s())
		{
			derr << "Malformed 32 bit image " << description << "\n";
			return;
		}
	}

	__forceinline void writeRLE(uchar* data, obinstream& s)
	{
		s.s().write((char*)data, 4);
	}

	__forceinline bool pixelCompare(uchar* lhs, uchar* rhs)
	{
		return (*(uint*)lhs) == (*(uint*)rhs);
	}
};

class PixelFunc8Bit
{
public:
	__forceinline void readRLE(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		char c;
		c = s.s().get();
		if (!s.s())
		{
			derr << "Malformed 8 bit image " << description << "\n";
			return;
		}

		memset(data, c, count);
	}

	__forceinline void readRaw(const std::string& description, uchar* data, ibinstream& s, int count)
	{
		s.s().read((char*)data, count);
		if (!s.s())
		{
			derr << "Malformed 8 bit image " << description << "\n";
			return;
		}
	}

	__forceinline void writeRLE(uchar* data, obinstream& s)
	{
		s.s().put(*data);
	}

	__forceinline bool pixelCompare(uchar* lhs, uchar* rhs)
	{
		return *lhs == *rhs;
	}
};

// Image functions
void Image::copy(const Image& dest) const
{
	copy(dest, Point2i(0, 0), Point2i(0, 0), size());
}

template <class SrcType, class DestType>
inline void pixelCopy(SrcType*, DestType*, const Image& srcImage, const Image& destImage, uchar* src, uchar* dest) {}

template<> inline void pixelCopy(PixelFunc8Bit*, PixelFunc24Bit*, const Image& srcImage, const Image& destImage, uchar* src, uchar* dest)
{
	uchar* palVal = srcImage.palette() + src[0] * 3;

	memcpy(dest, palVal, 3);
}

template<> inline void pixelCopy(PixelFunc8Bit*, PixelFunc32Bit*, const Image& srcImage, const Image& destImage, uchar* src, uchar* dest)
{
	uchar* palVal = srcImage.palette() + src[0] * 3;

	(*(int*)dest) = 0xFF000000 | ((int)palVal[2] << 16) | ((int)palVal[1] << 8) | (int)palVal[0];
}

template<> inline void pixelCopy(PixelFunc24Bit*, PixelFunc32Bit*, const Image& srcImage, const Image& destImage, uchar* src, uchar* dest)
{
	(*(int*)dest) = 0xFF000000 | ((int)src[2] << 16) | ((int)src[1] << 8) | (int)src[0];
}

template <class SrcType, class DestType>
inline void internalCopy(const Image& srcImage, const Image& destImage, const Point2i& destPos, const Point2i& srcMin, const Point2i& srcMax)
{
	uint srcStride = srcImage.stride();
	uint destStride = destImage.stride();
	uint copyLineSize = srcMax.x - srcMin.x;
	uint numLines = srcMax.y - srcMin.y;

	if (destPos.x + copyLineSize > (uint)destImage.size().x)
		copyLineSize = destImage.size().x - destPos.x;
	if (destPos.y + numLines > (uint)destImage.size().y)
		numLines = destImage.size().y - destPos.y;

	uchar* destData = destImage.data(destPos);
	uchar* srcData = srcImage.data(srcMin);
	int srcBytesPerPixel = srcImage.bytesPerPixel();
	int destBytesPerPixel = destImage.bytesPerPixel();

	for (uint y = 0; y < numLines; ++y)
	{
		for (uint i = 0; i < copyLineSize; i++)
		{
			pixelCopy<SrcType, DestType>(0, 0, srcImage, destImage, srcData, destData);
			srcData += srcBytesPerPixel;
			destData += destBytesPerPixel;
		}

		destData += destStride - copyLineSize * destImage.bytesPerPixel();
		srcData += srcStride - copyLineSize * srcImage.bytesPerPixel();
	}
}

void Image::copy(const Image& dest, const Point2i& destPos, const Point2i& srcMin, const Point2i& srcMax) const
{
	clamp(srcMin.x, 0, size().x);
	clamp(srcMin.y, 0, size().y);
	clamp(srcMax.x, 0, size().x);
	clamp(srcMax.y, 0, size().y);
	clamp(destPos.x, 0, dest.size().x);
	clamp(destPos.y, 0, dest.size().y);

	if (srcMax.x < srcMin.x || srcMax.y < srcMin.y)
		return;

	if (m_format == dest.m_format)
	{
		uchar* destData = dest.data(destPos);
		uchar* srcData = data(srcMin);

		// direct copy
		uint srcStride = size().x * bytesPerPixel();
		uint destStride = dest.size().x * bytesPerPixel();
		uint copyLineSize = (srcMax.x - srcMin.x);

		if (destPos.x + (copyLineSize / bytesPerPixel()) > (uint)dest.size().x)
			copyLineSize = dest.size().x - destPos.x;

		for (int y = srcMin.y; y < srcMax.y; ++y)
		{
			memcpy(destData, srcData, copyLineSize * bytesPerPixel());
			destData += destStride;
			srcData += srcStride;
		}
	}
	else
	{
		switch (m_format)
		{
		case BPP8:
			switch (dest.m_format)
			{
			case RGB24: internalCopy<PixelFunc8Bit, PixelFunc24Bit>(*this, dest, destPos, srcMin, srcMax); break;
			case ARGB32: internalCopy<PixelFunc8Bit, PixelFunc32Bit>(*this, dest, destPos, srcMin, srcMax); break;
			default: throwf("Can't copy Images - no format support");
			};
			break;
		case RGB24:
			switch (dest.m_format)
			{
			case ARGB32: internalCopy<PixelFunc24Bit, PixelFunc32Bit>(*this, dest, destPos, srcMin, srcMax); break;
			default: throwf("Can't copy Images - no format support");
			};
			break;
		default:
			throwf("Can't copy Images - no format support");
		};
	}
}

template <class T>
void Image::genericWriteRLEInternal(obinstream& s, T func) const
{
	uchar* seek = m_data;
	uchar* write = m_data;
	uchar* end = m_data + size().x * size().y * bytesPerPixel();
	uint perPixel = bytesPerPixel();
	uint lineCounter = 0;

	while (write != end)
	{
		uint distance = 0;

		// don't bother about raw packets for now - just write rle
		while (seek != end && func.pixelCompare(write, seek) && distance < 128 * perPixel && lineCounter < (uint)size().x)
		{
			seek += perPixel;
			distance = seek - write;
			lineCounter++;
		}

		// we have 127 identical pixels or have encountered a pixel change, encode it
		// write repetition bit
		uchar repetition;
		repetition = 0x80 | (uchar)((distance / perPixel) - 1);
		s << repetition;
		// write pixel
		func.writeRLE(write, s);

		// images aren't allowed to RLE encode over scanlines
		if (lineCounter >= (uint)size().x)
			lineCounter = 0;

		write = seek;
	}

	if (!s.s())
		throwf("Error occured while writing image");
}

template <class T>
void Image::genericReadInternal(const std::string& description, ibinstream& s, T func, bool bRLE)
{
	int remaining = size().x * size().y;
	m_data = new uchar[remaining * bytesPerPixel()];
	uchar* data = m_data;
	uint perPixel = bytesPerPixel();

	if (bRLE)
	{
		while (s.s() && remaining)
		{
			uchar repetitionByte = s.s().get();
			uchar pixelCount;
			if (!s.s())
			{
				derr << "Malformed image " << description << "\n";
				break;
			}

			pixelCount = (repetitionByte & 0x7F) + 1;
			remaining -= pixelCount;

			if (repetitionByte & 0x80)
			{
				// RLE packet
				func.readRLE(description, data, s, pixelCount);
			}
			else
			{
				// raw packet
				func.readRaw(description, data, s, pixelCount);
			}

			data += pixelCount * perPixel;
		}
	}
	else
	{
		s.s().read((char*)data, remaining * bytesPerPixel());
		if (!s.s())
			throwf("Bad read of uncompressed image");
	}
}

void Image::readPalette(ibinstream& s)
{
	m_palette = new uchar[256 * 3];

	s.s().read((char*)m_palette, 256 * 3);
}

void Image::genericRead(const std::string& description, ibinstream& s, bool bRLE)
{
	switch (m_format)
	{
	case BPP8:
		genericReadInternal(description, s, PixelFunc8Bit(), bRLE); break;
	case RGB24:
		genericReadInternal(description, s, PixelFunc24Bit(), bRLE); break;
	case ARGB32:
		genericReadInternal(description, s, PixelFunc32Bit(), bRLE); break;
	default:
		throwf("Can't read image - no format support");
	};
}

void Image::genericWriteRLE(obinstream& s) const
{
	switch (m_format)
	{
	case BPP8:
		genericWriteRLEInternal(s, PixelFunc8Bit()); break;
	case RGB24:
		genericWriteRLEInternal(s, PixelFunc24Bit()); break;
	case ARGB32:
		genericWriteRLEInternal(s, PixelFunc32Bit()); break;
	default:
		throwf("Can't write image - no format support");
	};
}

void Image::create(int width, int height, COMMONFORMAT format)
{
	if (width < 0 || height < 0)
		throwf("Image::create - Invalid dimensions");

	if (m_data)
		delete m_data;
	if (m_palette)
	{
		delete m_palette;
		m_palette = 0;
	}

	m_format = format;

	switch (format)
	{
	case RGB24:
		{
			m_data = new uchar[width * height * 3];
			memset(m_data, 0, width * height * 3);
			break;
		}
	case ARGB32:
		{
			m_data = new uchar[width * height * 4];

			uint dataSize = width * height;

			dword* ddata = (dword*)m_data;

			for (uint i = 0; i < dataSize; ++i)
				*ddata++ = 0xFF000000;

			break;
		}
	default:
		throw(ExceptionString("Image create - unsupported format."));
	};
}

size_t Image::dataBytes() const
{
	return stride() * size().y;
}

void Image::copy(uchar* dest, uint destStride) const
{
	uint srcStride = stride();
	uint copyLineSize = size().x;
	uint numLines = size().y;

	uchar* destData = dest;
	uchar* srcData = data();
	int srcBytesPerPixel = bytesPerPixel();
	int destBytesPerPixel = srcBytesPerPixel;

	for (uint y = 0; y < numLines; ++y)
	{
		memcpy(destData, srcData, srcBytesPerPixel * copyLineSize);

		destData += destStride + copyLineSize * destBytesPerPixel;
		srcData += srcStride + copyLineSize * srcBytesPerPixel;
	}
}
