// ------------------------------------------------------------------------------------------------
//
// SplitImage
// The image is split into as many square textures of size 'maxSize' as possible, then a right cap
// with a width of a power of two and height of the square texture block
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "SplitImage.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include <Standard/RawImage.h>
#include "UI/DialogMgr.h"

SplitImage::SplitImage(const Image& img, int maxSize)
{
	RawImage newImage;
	newImage.create(img.size().x, img.size().y, Image::ARGB32);
	img.copy(newImage);
	
	m_size = Point2((float)img.size().x, (float)img.size().y);

	// cut the image into 'maxSize' chunks
	for (uint y = 0; y < m_size.y; y += maxSize)
	{
		uint texHeight;
		uint roundedHeight;

		if (y + maxSize > m_size.y)
		{
			texHeight = (uint)m_size.y - y;
			roundedHeight = roundToPower(texHeight, maxSize);
		}
		else
		{
			texHeight = maxSize;
			roundedHeight = maxSize;
		}

		for (uint x = 0; x < m_size.x; x += maxSize)
		{
			uint texWidth;
			uint roundedWidth;

			if (x + maxSize > m_size.x)
			{
				texWidth = (uint)m_size.x - x;
				roundedWidth = roundToPower(texWidth, maxSize);
			}
			else
			{
				texWidth = maxSize;
				roundedWidth = maxSize;
			}

			IDirect3DTexture9* tex;
			D3DD().CreateTexture(roundedWidth, roundedHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, 0);

			if (!tex)
				throwf("Couldn't create SplitImage texture " + m_textures.size());

			D3DLOCKED_RECT r;
			tex->LockRect(0, &r, 0, 0);

			if (!r.pBits)
				throwf("Couldn't lock SplitImage");

			uchar* data = (uchar*)r.pBits;
			for (uint i = 0; i < texHeight; ++i)
			{
				memcpy(data, newImage.data(Point2i(x, y + i)), newImage.bytesPerPixel() * texWidth);
				data += r.Pitch;
			}

			tex->UnlockRect(0);

			m_textures.push_back(sEntry(Point2i(texWidth, texHeight), Point2((float)texWidth / roundedWidth, (float)texHeight / roundedHeight), tex));
		}
	}
}

uint SplitImage::roundToPower(uint val, uint maxSize)
{
	uint size;
	
	for (uint i = 1; i < 11; ++i)
	{
		size = (uint)pow(2, i);

		if (size >= val)
		{
			return size;
		}
	}

	return maxSize;
}

void SplitImage::draw(Point2 pos, const Point2& scale)
{
	float startX = pos.x;
	float startY = pos.y;
	float x = startX;
	float y = startY;

	D3DPainter& p = D3DPaint();
	p.reset();

	for (uint i = 0; i < m_textures.size(); ++i)
	{
		sEntry& e = m_textures[i];

		float xsize = e.size.x * scale.x;
		float ysize = e.size.y * scale.y;
		float x1 = x + xsize;
		float y1 = y + ysize;

		p.setFill(RGBA(1, 1, 1), e.tex, true);
		p.uv0 = Point2::ZERO;
		p.uv1 = e.uv;
		p.quad2D(x, y, x1, y1);
		p.draw();

		if (x1 - x > m_size.x * scale.x)
		{
			x = startX;
			y += ysize;
		}
		else
			x += xsize;
	}
}