// ------------------------------------------------------------------------------------------------
//
// TGA
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_TGA_H
#define STANDARD_TGA_H

#include "Standard/api.h"
#include "Math.h"
#include "Image.h"
class ibinstream;
class obinstream;
class Path;

class STANDARD_API TGA : public Image
{
public:
#pragma pack(push, 1) // tbd: is this pack directive still required?
	struct TGAHeader				// TGA file header
	{
		unsigned char	identLen;			// length of header comment
		unsigned char	colormap;			// 0 = no palette, else palette
		unsigned char	encoding;			// 1 = raw pal, 2 = raw rgb, 10 = RLE rgb
		unsigned short	palIndex;			// start of palette
		unsigned short	palLength;			// number of palette entries
		unsigned char	palSize;			// bits per palette entry
		unsigned short	xmin;				// top-left corner X
		unsigned short	ymin;				// top-left corner Y
		unsigned short	width;				// width of bitmap in pixels
		unsigned short	height;				// height of bitmap in pixels
		unsigned char	depth;				// bits per pixel
		unsigned char	desc;				// image attributes
	};
#pragma pack(pop)

	TGA();
	TGA(const Path& p);
	TGA(const std::string& description, ibinstream& stream);

	void create(int width, int height, COMMONFORMAT format);

	void read(const std::string& description, ibinstream& s);
	void write(obinstream& s) const;

	const TGAHeader& header() const { return m_header; }

	virtual DMath::Point2i size() const { return DMath::Point2i(m_header.width, m_header.height); }

	void colourKey(const DMath::RGBA& col, const DMath::RGBA& replacement);

private:
	void tga_read(const std::string& description, ibinstream& s);
	TGAHeader m_header;
};

#endif
