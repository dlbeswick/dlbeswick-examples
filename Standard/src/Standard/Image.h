// ------------------------------------------------------------------------------------------------
//
// Image
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/api.h"
#include "Math.h"

class ibinstream;
class obinstream;


class STANDARD_API Image
{
public:
	enum COMMONFORMAT
	{
		BPP8 = 0,
		RGB24,
		ARGB32,
		FORMAT_NUM
	};

	Image()
	{
		m_data = 0;
		m_palette = 0;
	}

	virtual ~Image()
	{
		if (m_data)
			delete m_data;
		if (m_palette)
			delete m_palette;
	}

	virtual void create(int width, int height, COMMONFORMAT format);

	virtual void read(const std::string& description, ibinstream& s) = 0;
	virtual void write(obinstream& s) const = 0;

	virtual uchar* data() const { return m_data; }
	virtual uchar* data(const DMath::Point2i& p) const { return data(p.x, p.y); }
	virtual uchar* data(uint x, uint y) const { return data() + calcOffset(x, y); }
	virtual size_t dataBytes() const;
	virtual uchar* palette() const { return m_palette; }
	virtual DMath::Point2i size() const = 0;
	virtual int stride() const { return size().x * bytesPerPixel(); }

	void copy(const Image& dest) const;
	void copy(const Image& dest, const DMath::Point2i& destPos, const DMath::Point2i& srcMin, const DMath::Point2i& srcMax) const;
	void copy(uchar* dest, uint destStride = UINT_MAX) const;

	int bytesPerPixel() const { return m_bytesPerPixel[(int)m_format]; }

	COMMONFORMAT format() const { return m_format; }

protected:
	void genericRead(const std::string& description, ibinstream& s, bool bRLE);
	void genericWriteRLE(obinstream& s) const;

	virtual void readPalette(ibinstream& s);

	uint calcOffset(const DMath::Point2i& p) const { return calcOffset(p.x, p.y); }
	uint calcOffset(uint x, uint y) const { return size().x * bytesPerPixel() * y + x * bytesPerPixel(); }

	COMMONFORMAT m_format;
	static int m_bytesPerPixel[FORMAT_NUM];
	uchar* m_data;
	uchar* m_palette;

private:
	template <class T>
	void genericReadInternal(const std::string& description, ibinstream& s, T func, bool bRLE);

	template <class T>
	void genericWriteRLEInternal(obinstream& s, T func) const;
};

inline ibinstream& operator >> (ibinstream& s, Image& img) { img.read("Stream", s); return s; }
inline obinstream& operator << (obinstream& s, const Image& img) { img.write(s); return s; }
