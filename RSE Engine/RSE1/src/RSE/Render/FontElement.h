// ---------------------------------------------------------------------------------------------------------
// 
// FontElement
// 
// ---------------------------------------------------------------------------------------------------------
#ifndef RSE_FONTELEMENT_H
#define RSE_FONTELEMENT_H

#include "RSE/RSE.h"
#include "Standard/Streamable.h"
#include "Standard/Math.h"
#include "Standard/PtrD3D.h"
class D3DPainter;

class RSE_API FontElement : public Streamable
{
	USE_RTTI(FontElement, FontElement);
public:
	FontElement(const std::string& name);
	virtual ~FontElement();

	bool operator== (const FontElement& rhs) const;
	bool operator!= (const FontElement& rhs) const;

	virtual void construct();
	virtual FontElement* clone(const std::string& newName) = 0;

	virtual const std::string& name() const { return m_name; }
	virtual float height() const = 0;
	virtual const PtrD3D<IDirect3DTexture9>& texture() { return m_pTexture; }
	virtual float stringWidth(const std::string &text) const;

	virtual void setScale(float scale) { m_scale = scale; }
	
	virtual void write(
		D3DPainter& paint, 
		const std::string &text, 
		float x, 
		float y, 
		float z = 1, 
		const RGBA& colour = RGBA(1,1,1)
	) const;

	virtual void worldWrite(
		class Camera& cam, 
		D3DPainter& paint, 
		const std::string &text, 
		const Point3 &pos, 
		const RGBA& colour = RGBA(1,1,1)
	) const;

protected:
	typedef stdext::hash_map<int, float> KerningAmtMap;

	struct CharacterInfo
	{
		CharacterInfo();

		Point2 uvMin;
		Point2 uvMax;
		float width;
		KerningAmtMap kerning;
		
		inline float startAdjustment(const std::vector<CharacterInfo>& infos, const std::string& string, int charIdx) const;
	};

	typedef std::vector<CharacterInfo> CharacterInfoList;

	virtual PtrD3D<IDirect3DTexture9> createTexture() = 0;
	virtual const CharacterInfoList& characterInfo() const = 0;

	virtual void writeInternal(D3DPainter& paint, const std::string &text, float x, float y, float z, const Matrix4& xform, const RGBA& colour) const;

	float						m_scale;
	std::string					m_name;

	friend class FontResource;

private:
	PtrD3D<IDirect3DTexture9>	m_pTexture;
};

float FontElement::CharacterInfo::startAdjustment(const std::vector<CharacterInfo>& infos, const std::string& string, int charIdx) const
{
	if (charIdx == 0)
		return 0.0f;

	const KerningAmtMap& prevCharKerning = infos[string[charIdx-1]].kerning;
	KerningAmtMap::const_iterator it1 = prevCharKerning.find(string[charIdx]);

	if (it1 != prevCharKerning.end())
	{
		return it1->second;
	}

	return 0.0f;
}

#endif
