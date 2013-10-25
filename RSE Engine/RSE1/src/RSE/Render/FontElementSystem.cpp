#include "pch.h"
#include "FontElementSystem.h"
#include "RSE/Exception/Video.h"
#include "RSE/Render/SDeviceD3D.h"
#include "RSE/Render/D3DPainter.h"
#include "RSE/Render/Materials/MaterialSolid.h"
#include "Standard/D3DHelp.h"

IMPLEMENT_RTTI(FontElementSystem);

FontElementSystem::FontElementSystem(
		const std::string& face, 
		const std::string& name, 
		uint size, 
		uint texSize, 
		uint weight, 
		bool bItalic
	) :
	FontElement(name),
	m_face(face),
	m_size(size),
	m_texSize(texSize),
	m_weight(weight),
	m_bItalic(bItalic)
{
}

// createTexture
PtrD3D<IDirect3DTexture9> FontElementSystem::createTexture()
{
	// tbd: size texture only as necessary

	// Create font
	HFONT font = CreateFont(
		m_size,
		0,
		0,
		0,
		m_weight,
		m_bItalic,
		false,
		false,
		ANSI_CHRSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		m_face.c_str()
	);

	if (!font)
		return 0;

	IDirect3DTexture9* pTex = 0;

	// Make a texture
	if (DXFAIL(D3DD().CreateTexture(m_texSize, m_texSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
			&pTex, 0)))
		return 0;

	IDirect3DSurface9 *pOldTarget;
	if (DXFAIL(D3DD().GetRenderTarget(0, &pOldTarget)))
		return 0;

	IDirect3DSurface9 *pOldDepth = 0;
	D3DD().GetDepthStencilSurface(&pOldDepth);
	D3DD().SetDepthStencilSurface(0);

	// Render SFont to texture
	IDirect3DSurface9 *pTexSurface;
	if (DXFAIL(pTex->GetSurfaceLevel(0, &pTexSurface)))
		return 0;

	if (DXFAIL(D3DD().SetRenderTarget(0, pTexSurface)))
		return 0;

	D3DD().BeginScene();

	// clear texture
	Matrix4 mat = Matrix4::IDENTITY;

	D3DD().SetTransform(D3DTS_WORLD1, mat);
	mat.scale((float)m_texSize, (float)m_texSize, 1);
	D3DD().SetTransform(D3DTS_PROJECTION, mat);
	D3DPaint().reset();

	MaterialSolid m(RGBA(1,1,1,0),RGBA(1,1,1,0),RGBA(0,0,0,0),RGBA(1,1,1,0));
	D3DPaint().setFill(m);

	D3D().alpha(false);
	D3DPaint().quad(Quad(Point3(-1, -1, 0), Point3(1, -1, 0), Point3(1, 1, 0), Point3(-1, 1, 0)));
	D3DPaint().draw();

	D3DPaint().setFill(RGBA(1,1,1));

	// select SFont into hdc to get widths
	HDC hdc;
	DX_ENSURE(pTexSurface->GetDC(&hdc));

	SetTextColor(hdc, 0xffffffff);

	RECT rect;
	// leave a gap for bilinear filtering
	rect.left = 1;
	rect.top = 1;
	rect.right = 1;
	rect.bottom = m_size;

	_characterInfo.resize(256);

	for (uint i = 32; i < 128; i++)
	{
		CharacterInfo& info = _characterInfo[i];

		ABC abc;
		GetCharABCWidths(hdc, i, i, &abc);
		// tbd: store abc sizes? that'd fix spaces.
		// leave a gap for bilinear filtering
		int width = abc.abcA + abc.abcB + abc.abcC;

		rect.right = rect.left + width;
		if (rect.right >= (int)m_texSize)
		{
			rect.top += m_size;
			rect.bottom += m_size;
			// leave a gap for bilinear filtering
			rect.left = 1;
			rect.right = width;
		}

		if (rect.bottom >= (int)m_texSize)
			break;

		Point2& coordsMin = info.uvMin;
		Point2& coordsMax = info.uvMax;

		coordsMin.x = (float)rect.left / m_texSize;
		coordsMin.y = (float)rect.top / m_texSize;
		coordsMax.x = (float)rect.right / m_texSize;
		coordsMax.y = (float)rect.bottom / m_texSize;

		info.width = width / 640.0f;

		char c = i;
		RECT drawRect = rect;

		TextOut(hdc, drawRect.left, drawRect.top, &c, 1);

		// leave a gap for bilinear filtering
		rect.left += width + 1;
	}

	D3DD().EndScene();

	DXFAIL(D3DD().SetRenderTarget(0, pOldTarget));

	storeKerning(hdc);

	// generate mipmaps
	// fix: usage_automipmaps?
	//DXFAIL(D3DXFilterTexture(pTex, 0, 0, D3DX_FILTER_TRIANGLE));

	D3DD().SetDepthStencilSurface(pOldDepth);
	if (pOldDepth)
		pOldDepth->Release();
	pOldTarget->Release();
	pTexSurface->Release();

	return pTex;
}

void FontElementSystem::storeKerning(HDC hdc)
{
	int numKerningPairs = GetKerningPairs(hdc, 0, 0);

	KERNINGPAIR* winPairs = new KERNINGPAIR[numKerningPairs];
	GetKerningPairs(hdc, numKerningPairs, winPairs);

	for (int i = 0; i < numKerningPairs; ++i)
	{
		KERNINGPAIR& pair = winPairs[i];
		_characterInfo[pair.wFirst].kerning[pair.wSecond] = pair.iKernAmount / 640.0f;
	}

	delete[] winPairs;
}

float FontElementSystem::height() const
{
	return m_size / 640.0f;
}

FontElement* FontElementSystem::clone(const std::string& newName)
{
	FontElementSystem* f = new FontElementSystem(*this);
	f->m_name = newName;

	return f;
}

const FontElement::CharacterInfoList& FontElementSystem::characterInfo() const
{
	return _characterInfo;
}
