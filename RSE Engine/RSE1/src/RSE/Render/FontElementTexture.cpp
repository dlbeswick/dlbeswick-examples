#include "pch.h"
#include "FontElementTexture.h"
#include "PathResource.h"
#include "Exception/Video.h"
#include "Render/SDeviceD3D.h"
#include "Standard/TGA.h"

IMPLEMENT_RTTI(FontElementTexture);

#define METADATA FontElementTexture
STREAM
	STREAMVAR(characters);
	STREAMVAR(characterSize);
	STREAMVAR(spaceWidth);
	STREAMVAR(textureName);
	STREAMVAR(uppercaseOnly);
END_STREAM

FontElementTexture::FontElementTexture(const std::string& name) :
	FontElement(name)
{
}

PtrD3D<IDirect3DTexture9> FontElementTexture::createTexture()
{
	PathResource texturePath(textureName);
	TGA src(texturePath);

	uint characterIdx = 0;

	TGA colourKeyed;
	colourKeyed.create(src.size().x, src.size().y, Image::ARGB32);
	src.copy(colourKeyed);
	colourKeyed.colourKey(RGBA(1, 0, 1), RGBA(1, 1, 1, 0));

	PtrD3D<IDirect3DTexture9> result = D3D().loadTexture(colourKeyed, true, true);

	IDirect3DSurface9* surf;
	DX_ENSURE(result->GetSurfaceLevel(0, &surf));
	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);
	surf->Release();

	Point2 textureSize((float)desc.Width, (float)desc.Height);

	_characterInfo.resize(256);

	// start at 1,1 for bilinear filtering
	Point2 pos;
	for (pos.y = 1; pos.y < src.size().y && characterIdx < characters.size(); pos.y += characterSize.y + 1)
	{
		// +1 for bilinear filtering
		for (pos.x = 1; pos.x < src.size().x && characterIdx < characters.size(); pos.x += characterSize.x + 1)
		{
			int character = clamp((int)characters[characterIdx++], 0, 255);
			CharacterInfo& info = _characterInfo[character];

			info.uvMin = pos / textureSize;
			info.uvMax = (pos + characterSize) / textureSize;
			info.width = characterSize.x / 640.0f;
		}
	}

	if (uppercaseOnly)
	{
		for (int i = 'a'; i <= 'z'; ++i)
		{
			_characterInfo[i] = _characterInfo['A' + i - 'a'];
		}
	}

	CharacterInfo& spaceInfo = _characterInfo[' '];
	spaceInfo.width = spaceWidth / 640.0f;

	return result;
}

float FontElementTexture::height() const
{
	return characterSize.y / 640.0f;
}

const FontElement::CharacterInfoList& FontElementTexture::characterInfo() const
{
	return _characterInfo;
}
