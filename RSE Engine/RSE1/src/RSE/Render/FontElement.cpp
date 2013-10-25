// ---------------------------------------------------------------------------------------------------------
// 
// FontElement
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "FontElement.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include "Render/BillBoarder.h"
#include "Render/Materials/MaterialSolid.h"
#include "Render/VertexBuffer.h"
#include "Standard/D3DHelp.h"

IMPLEMENT_RTTI(FontElement);

// font resource
class RSE_API FontResource : public ID3DResource
{
public:
	FontResource(FontElement& _font) :
		font(_font)
	{
	}

	FontElement& font;

	virtual void release(SDeviceD3D& device)
	{
		font.m_pTexture.release();
	}

	virtual void create(SDeviceD3D& device)
	{
		font.m_pTexture.remap(font.createTexture());
	}
};

FontElement::CharacterInfo::CharacterInfo() :
	uvMin(0, 0),
	uvMax(0, 0),
	width(5.0f / 640.0f)
{
}

// construct
FontElement::FontElement(const std::string& name) :
	m_scale(1),
	m_pTexture(0),
	m_name(name)
{
}

FontElement::~FontElement()
{
}

void FontElement::construct()
{
	FontResource* f = new FontResource(*this);

	// note resource
	D3D().addResource(f);

	m_pTexture = f->font.texture();
	if (!m_pTexture)
		throwf("Couldn't create font " + name());
}

// stringWidth
float FontElement::stringWidth(const std::string &text) const
{
	float width = 0;

	const CharacterInfoList& infos = characterInfo();

	for (uint i = 0; i < text.size(); i++)
	{
		const CharacterInfo& info = infos[text[i]];
		width += info.width + info.startAdjustment(infos, text, i);
	}

	return width * m_scale;
}

void FontElement::writeInternal(D3DPainter& paint, const std::string &text, float x, float y, float z, const Matrix4& xform, const RGBA& colour) const
{
	if (text.empty())
		return;
		
	paint.setFill(colour, m_pTexture);

    VertexBufferStream& vb = paint.vbTri();
	vb.ensure(text.size() * 6);

	D3DPainter::VERT vert[6];
	vert[0].c = RGBA(1,1,1);
	vert[1].c = RGBA(1,1,1);
	vert[2].c = RGBA(1,1,1);
	vert[3].c = RGBA(1,1,1);
	vert[4].c = RGBA(1,1,1);
	vert[5].c = RGBA(1,1,1);

	float height = this->height() * m_scale;

	const CharacterInfoList& infos = characterInfo();

	for (uint i = 0; i < text.size(); i++)
	{
		const CharacterInfo& info = infos[text[i]];

		x += info.startAdjustment(infos, text, i);

		float vx = (float)x;
		float vy = (float)y;
		float vx1 = (float)x + info.width * m_scale;
		float vy1 = (float)y + height;
		const Point2 &txMin = info.uvMin;
		const Point2 &txMax = info.uvMax;

		vert[0].pos = Point3(vx, vy, z) * xform;
		vert[0].tex[0] = txMin.x;
		vert[0].tex[1] = txMin.y;

		vert[1].pos = Point3(vx1, vy, z) * xform;
		vert[1].tex[0] = txMax.x;
		vert[1].tex[1] = txMin.y;

		vert[2].pos = Point3(vx1, vy1, z) * xform;
		vert[2].tex[0] = txMax.x;
		vert[2].tex[1] = txMax.y;

		vert[3].pos = Point3(vx, vy, z) * xform;
		vert[3].tex[0] = txMin.x;
		vert[3].tex[1] = txMin.y;

		vert[4].pos = Point3(vx1, vy1, z) * xform;
		vert[4].tex[0] = txMax.x;
		vert[4].tex[1] = txMax.y;

		vert[5].pos = Point3(vx, vy1, z) * xform;
		vert[5].tex[0] = txMin.x;
		vert[5].tex[1] = txMax.y;

		paint.calcNormals(vert, 2);
		vb.insert(vert, 6);

		x += info.width * m_scale;
	}
}

void FontElement::write(D3DPainter& paint, const std::string &text, float x, float y, float z, const RGBA& colour) const
{
	writeInternal(paint, text, x, y, z, Matrix4::IDENTITY, colour);
}

// worldWrite
void FontElement::worldWrite(Camera& cam, class D3DPainter& painter, const std::string& text, const Point3& pos, const RGBA& colour) const
{
	// note to me: cam.getBase() is a mapping from another coord system (max) to D3D coords
	// font is normally expressed in screen coords

	Matrix4 mat = cam.base();
	mat(1, 0) *= -1;	// y positive is up for screen, reverse for world
	mat(1, 1) *= -1;
	mat(1, 2) *= -1;
	mat(3, 0) = pos.x;
	mat(3, 1) = pos.y;
	mat(3, 2) = pos.z;

	writeInternal(painter, text, 0, 0, 0, mat, colour);
}

bool FontElement::operator== (const FontElement& rhs) const
{
	return name() == rhs.name();
}

bool FontElement::operator!= (const FontElement& rhs) const
{
	return !(*this == rhs);
}
