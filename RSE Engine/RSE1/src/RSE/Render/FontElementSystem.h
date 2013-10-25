#ifndef RSE_FONTELEMENTSYSTEM_H
#define RSE_FONTELEMENTSYSTEM_H

#include "RSE/RSE.h"
#include "RSE/Render/FontElement.h"

class RSE_API FontElementSystem : public FontElement
{
	USE_RTTI(FontElementSystem, FontElement);
public:
	FontElementSystem(const std::string& face, const std::string& name, uint size, uint texSize, uint weight, 
						bool bItalic);
	
	virtual FontElement* clone(const std::string& newName);
	virtual float height() const;

protected:
	virtual const CharacterInfoList& characterInfo() const;
	virtual PtrD3D<IDirect3DTexture9> createTexture();
	virtual void storeKerning(HDC hdc);

	std::string		m_face;
	uint			m_size;
	uint			m_texSize;
	uint			m_weight; 
	bool			m_bItalic;

	CharacterInfoList _characterInfo;
};

#endif
