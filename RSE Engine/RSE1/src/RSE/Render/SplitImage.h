#ifndef RSE_SPLITIMAGE_H
#define RSE_SPLITIMAGE_H

#include "RSE/RSE.h"
#include "Standard/Math.h"
#include "Standard/PtrD3D.h"
class Image;

/// An image composed of several smaller images of a given size.
class RSE_API SplitImage
{
public:
	SplitImage(const Image& img, int maxSize = 512);

	void draw(Point2 pos, const Point2& scale = Point2(1.0f, 1.0f));

	const Point2& size() const { return m_size; }

protected:
	struct sEntry
	{
		sEntry(const Point2i& _size, const Point2& _uv, const PtrD3D<IDirect3DTexture9>& _tex) :
			size(_size), uv(_uv), tex(_tex)
		{}

		Point2i size;
		Point2 uv;
		PtrD3D<IDirect3DTexture9> tex;
	};

	std::vector<sEntry> m_textures;

	uint roundToPower(uint val, uint maxSize);

	Point2 m_size;
};

#endif
