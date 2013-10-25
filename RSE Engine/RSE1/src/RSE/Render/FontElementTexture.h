#ifndef RSE_FONTELEMENTTEXTURE_H
#define RSE_FONTELEMENTTEXTURE_H

#include "RSE/RSE.h"
#include "RSE/Render/FontElement.h"

class RSE_API FontElementTexture : public FontElement
{
	USE_RTTI(FontElementTexture, FontElement);
	USE_STREAMING;
public:
	FontElementTexture(const std::string& name);

	virtual FontElement* clone(const std::string& newName) { throw(0); }
	virtual float height() const;

protected:
	virtual PtrD3D<IDirect3DTexture9> createTexture();
	virtual const CharacterInfoList& characterInfo() const;

	std::string characters;
	Point2i characterSize;
	float spaceWidth;
	std::string textureName;
	bool uppercaseOnly;

	CharacterInfoList _characterInfo;
};

#endif
