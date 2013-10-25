// ---------------------------------------------------------------------------------------------------------
// 
// SDeviceD3D
// 
// ---------------------------------------------------------------------------------------------------------
#include "pch.h"

#include "AppBase.h"
#include "D3DPainter.h"
#include "ID3DResource.h"
#include "SDeviceD3D.h"
#include "RenderContext.h"
#include "Exception/Video.h"
#include "Materials/MaterialTexture.h"
#include "UI/DialogMgr.h"
#include "Standard/binbufstream.h"
#include <Standard/Exception/Filesystem.h>
#include "Standard/FileMgr.h"
#include "Standard/Mapping.h"
#include "Standard/RawImage.h"
#include "Standard/Rect.h"
#include "Standard/STLHelp.h"
#include "Standard/TGA.h"

// resources
class RSE_API TextureResource : public ID3DResource
{
public:
	TextureResource(const std::string& _filename, bool _bMipMap, PtrD3D<IDirect3DTexture9> _resource) :
		filename(_filename),
		bMipMap(_bMipMap),
		resource(_resource)
	{
	}

	std::string					filename;
	bool						bMipMap;
	PtrD3D<IDirect3DTexture9>	resource;

	virtual void release(SDeviceD3D& device)
	{
		resource.release();
	}

	virtual void create(SDeviceD3D& device)
	{
		PtrD3D<IDirect3DTexture9> newTexture = device.loadTexture(filename, bMipMap);
		resource.remap(newTexture);
	}
};

class RSE_API TextureResourceFromRes : public ID3DResource
{
public:
	TextureResourceFromRes(const std::string& _resname, const char* _restype, bool _bMipMap, PtrD3D<IDirect3DTexture9> _resource) :
		resname(_resname),
		restype(_restype),
		bMipMap(_bMipMap),
		resource(_resource)
	{
	}

	std::string					resname;
	const char*					restype;
	bool						bMipMap;
	PtrD3D<IDirect3DTexture9>	resource;

	virtual void release(SDeviceD3D& device)
	{
		resource->Release();
	}

	virtual void create(SDeviceD3D& device)
	{
		PtrD3D<IDirect3DTexture9> newTexture = device.loadTextureFromResource(resname, restype, bMipMap);
		resource.remap(newTexture);
	}
};

// construct
SDeviceD3D::SDeviceD3D() :
	_device(0),
	m_bRestoring(false),
	painter(0),
	m_context(0),
	m_defaultContext(0),
	_mixedMode(false),
	m_viewMin(0, 0),
	m_viewMax(1, 1)
{
	painter = new D3DPainter();

	// init d3d object
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D)
	{
		throwf("Direct3D initalisation failure");
	}
}

void SDeviceD3D::destroy()
{
	_nullTexture = 0;

	delete m_defaultContext;
	m_defaultContext = 0;

	m_resources.flush();
	for (uint i = 0; i < m_resources.size(); ++i)
	{
		m_resources[i]->release(*this);
	}
	m_resources.clear();

	freeDX(_device);
	freeDX(m_pD3D);
}

SDeviceD3D::~SDeviceD3D()
{
	destroy();
	if (painter)
		delete painter;
}

void SDeviceD3D::recreateDevice()
{
	D3D().newDevice(_lastPresentParameters, true);
}

// newDevice
void SDeviceD3D::newDevice(bool bWindowed, int resX, int resY, int winResX, int winResY, bool b32BitDepthBuffer)
{
	// should only call this once -- then call recreateDevice or toggleFullscreen
	assert(!_device);

	D3DFORMAT depthFormat;
	
	if (b32BitDepthBuffer)
		depthFormat = D3DFMT_D32;
	else
		depthFormat = D3DFMT_D16;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed   = bWindowed;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = depthFormat;
	d3dpp.BackBufferCount = 1;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	m_screenSize.x = resX;
	m_screenSize.y = resY;
	m_winSize.x = winResX;
	m_winSize.y = winResY;

	if (bWindowed)
	{
		d3dpp.BackBufferWidth = m_winSize.x;
		d3dpp.BackBufferHeight = m_winSize.y;
	}
	else
	{
		d3dpp.BackBufferWidth = m_screenSize.x;
		d3dpp.BackBufferHeight = m_screenSize.y;
	}

	newDevice(d3dpp, false);
}

bool SDeviceD3D::supportsHardwareVP() const
{
	return _mixedMode;
}

void SDeviceD3D::newDevice(D3DPRESENT_PARAMETERS& d3dpp, bool isRecreating)
{
	D3DDISPLAYMODE d3ddm;
	DX_ENSURE(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm));

	d3dpp.BackBufferFormat = d3ddm.Format;

	if (_device)
	{
		DX_ENSURE(_device->Reset(&d3dpp));
	}
	else
	{
		int result = D3DERR_INVALIDCALL;

		if (AppBase().options().get("Render", "HardwareVertexProcessing", true))
		{
			result = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AppBase().hWnd(),
										D3DCREATE_MIXED_VERTEXPROCESSING,
										&d3dpp, &_device);

			_mixedMode = result == D3D_OK;
		}

		if (result != D3D_OK)
		{
			result = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, AppBase().hWnd(),
											D3DCREATE_SOFTWARE_VERTEXPROCESSING,
											&d3dpp, &_device);
			_mixedMode = false;
		}

#if _DEBUG
		if (result != D3D_OK)
		{
			result = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, AppBase().hWnd(),
											D3DCREATE_SOFTWARE_VERTEXPROCESSING,
											&d3dpp, &_device);
		}
#endif

		if (result != D3D_OK)
		{
			EXCEPTIONSTREAM(ExceptionD3D(result), "Couldn't create a Direct3D device in either 'mixed' or 'software' vertex processing modes.");
		}
	}

	D3DDEVICE_CREATION_PARAMETERS params;
	_device->GetCreationParameters(&params);

	RECT rect;
	GetWindowRect(params.hFocusWindow, &rect);
	rect.right = rect.left + d3dpp.BackBufferWidth;
	rect.bottom = rect.top + d3dpp.BackBufferHeight;
	AdjustWindowRectEx(&rect, GetWindowLong(params.hFocusWindow, GWL_STYLE), false, GetWindowLong(params.hFocusWindow, GWL_EXSTYLE));
	SetWindowPos(params.hFocusWindow, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0);

	_device->GetDeviceCaps(&m_caps);

	if (!isRecreating)
	{
		// enable mipmapping / linear filtering
		D3D().texFilter(true, true);

		// clamp textures
		_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		_device->SetRenderState(D3DRS_LIGHTING, true);

		// light
		D3DLIGHT9 light;
		ZeroMemory( &light, sizeof(D3DLIGHT9) );
		light.Type = D3DLIGHT_POINT;
		light.Diffuse.r  = 1.0f;
		light.Diffuse.g  = 1.0f;
		light.Diffuse.b  = 1.0f;

		light.Specular.r = 1.0f;
		light.Specular.g = 1.0f;
		light.Specular.b = 1.0f;

		light.Position.x = 10;
		light.Position.y = -10;
		light.Position.z = 8;
		light.Attenuation1 = 0.001f;
		light.Range = 5000.0f;

		for (uint i = 0; i < 8; ++i)
		{
			D3DD().SetLight(i, &light);
			D3DD().LightEnable(i, i==0);
		}

		D3DMATERIAL9 m;

		m.Diffuse = RGBA(1,1,1);
		m.Specular = RGBA(1,1,1);
		m.Ambient = RGBA(1,1,1);
		m.Emissive = RGBA(1,1,1);
		m.Power = 1;
		
		D3DD().SetMaterial(&m);
		D3DD().SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
		D3DD().SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
		D3DD().SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
		D3DD().SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		D3DD().SetRenderState(D3DRS_COLORVERTEX, false);

		markDefaultContext();

		_device->BeginScene();
		_device->Clear(0, 0, D3DCLEAR_TARGET, RGBA(0,0,0,1), 0, 0);
		_device->EndScene();
		_device->Present(0, 0, 0, 0);
	}
	else
	{
		if (d3dpp.Windowed != _lastPresentParameters.Windowed)
			DialogMgr().onFullscreenToggle(d3dpp.Windowed != 0);
	}

	if (context())
		setContext(*context(), true);

	_lastPresentParameters = d3dpp;
}

// toggleFullscreen
void SDeviceD3D::toggleFullscreen()
{
	_lastPresentParameters.Windowed ^= 1;
	newDevice(_lastPresentParameters, true);
}

// zbuffer
void SDeviceD3D::zbuffer(bool bUse)
{
	_device->SetRenderState(D3DRS_ZENABLE, bUse);
	_device->SetRenderState(D3DRS_ZWRITEENABLE, bUse);
}


// texFilter
void SDeviceD3D::texFilter(bool bUseMin, bool bUseMag)
{
	int filter;

	if (bUseMag)
		filter = D3DTEXF_LINEAR;
	else
		filter = D3DTEXF_POINT;

	_device->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);
	_device->SetSamplerState(0, D3DSAMP_MIPFILTER, filter);

	if (bUseMin)
		filter = D3DTEXF_LINEAR;
	else
		filter = D3DTEXF_POINT;

	_device->SetSamplerState(0, D3DSAMP_MINFILTER, filter);
}


// wireframe
void SDeviceD3D::wireframe(bool bUse)
{
	int fillmode;

	if (bUse)
		fillmode = D3DFILL_WIREFRAME;
	else
		fillmode = D3DFILL_SOLID;
	
	_device->SetRenderState(D3DRS_FILLMODE, fillmode);
}


// loadTexture
PtrD3D<IDirect3DTexture9> SDeviceD3D::loadTexture(const Image& image, bool bMipMap, bool makePowerOfTwo)
{
	double powWidth = pow((double)image.size().x, 0.5);
	double powHeight = pow((double)image.size().y, 0.5);

	bool isPowerOfTwo = powWidth == (int)powWidth && powHeight == (int)powHeight;

	IDirect3DTexture9* tex;

	bool ownedImage;
	Image* srcImage;

	D3DFORMAT format;
	switch (image.format())
	{
	case Image::RGB24:
		format = D3DFMT_X8R8G8B8;
		srcImage = new RawImage;
		srcImage->create(image.size().x, image.size().y, Image::ARGB32);
		image.copy(*srcImage);
		ownedImage = true;
		break;
	case Image::ARGB32:
		format = D3DFMT_A8R8G8B8;
		srcImage = (Image*)&image;
		ownedImage = false;
		break;
	case Image::BPP8:
        format = D3DFMT_A8R8G8B8;
		srcImage = new RawImage;
		srcImage->create(image.size().x, image.size().y, Image::ARGB32);
		image.copy(*srcImage);
		ownedImage = true;
		break;
	default:
		EXCEPTIONSTREAM(ExceptionVideo(), "Texture creation failed, unsupported texture format '" << image.format() << "'");
	}

	if (!isPowerOfTwo)
	{
		if (!makePowerOfTwo)
		{
			EXCEPTIONSTREAM(ExceptionVideo(), "Textures must be a power of two, but image size was " << image.size());
		}
		else
		{
			Point2i newSize(0, 0);

			for (float i = 0; i != 10; ++i)
			{
				float newX = powf(2.0f, i);
				if (newX >= srcImage->size().x)
				{
					newSize.x = (int)newX;
					break;
				}
			}

			for (float i = 0; i != 10; ++i)
			{
				float newY = powf(2.0f, i);
				if (newY >= srcImage->size().x)
				{
					newSize.y = (int)newY;
					break;
				}
			}

			if (newSize.x == 0 || newSize.y == 0)
				EXCEPTIONSTREAM(ExceptionVideo(), "Textures must be a power of two, and image size was larger than the maximum allowable size (image is " << image.size() << ")");

			RawImage* resized = new RawImage;
			resized->create(newSize.x, newSize.y, Image::ARGB32);
			memset(resized->data(), 0, resized->dataBytes());
			srcImage->copy(*resized);

			if (ownedImage)
				delete srcImage;

			srcImage = resized;
			ownedImage = true;
		}
	}

	int usage;
	if (bMipMap)
		usage = D3DUSAGE_AUTOGENMIPMAP;
	else
		usage = 0;

	DX_ENSURE(_device->CreateTexture(srcImage->size().x, srcImage->size().y, 1, usage, format, D3DPOOL_MANAGED, &tex, 0));

	D3DLOCKED_RECT rect;
	DX_ENSURE(tex->LockRect(0, &rect, 0, 0));

	char* bits = (char*)rect.pBits;
	uchar* src = srcImage->data();
	uchar* end = src + srcImage->stride() * srcImage->size().y;

	for ( ; src < end; bits += rect.Pitch, src += srcImage->stride())
	{
		memcpy(bits, src, image.stride());
	}

	tex->UnlockRect(0);

	if (ownedImage)
		delete srcImage;

	return tex;
}

PtrD3D<IDirect3DTexture9> SDeviceD3D::loadTexture(const Path& fName, bool bMipMap, bool makePowerOfTwo)
{
	try
	{
		return loadTexture(TGA(fName), bMipMap, makePowerOfTwo);
	}
	catch(ExceptionFilesystem&)
	{
		try
		{
			return loadTextureFromResource("missing", RT_BITMAP);
		}
		catch (ExceptionString&)
		{
			throw;
		}
	}
}

// loadTextureFromResource
PtrD3D<IDirect3DTexture9> SDeviceD3D::loadTextureFromResource(const std::string& resName, const char* resType, bool bMipMap, bool makePowerOfTwo)
{
	HRSRC findRes = FindResource(g_hRseDLL, resName.c_str(), resType);
	if (!findRes)
		throwf("Couldn't find resource  '" + resName + "'");

	HGLOBAL resHandle = LoadResource(g_hRseDLL, findRes);
	if (!resHandle)
		throwf("Couldn't load resource  '" + resName + "'");

	// note: the resource handle need not be freed
	LPVOID data = LockResource(resHandle);
	if (!data)
		throwf("Couldn't lock resource  '" + resName + "'");

	int mipLevels = 0;
	if (!bMipMap)
		mipLevels = 1;

	ibinbufstream stream((char*)data);

	TGA tga(resName, stream);

	return loadTexture(tga, bMipMap, makePowerOfTwo);
}

// isDeviceLost
bool SDeviceD3D::isDeviceLost()
{
	if (_device)
	{
		return _device->TestCooperativeLevel() != D3D_OK;
	}

	return true;
}

// onDeviceLost
void SDeviceD3D::onDeviceLost()
{
	m_bRestoring = true;

	m_resources.flush();

	// free resources
	for (uint i = 0; i < m_resources.size(); ++i)
	{
		m_resources[i]->release(*this);
	}

	// reset device
	newDevice(_lastPresentParameters, true);

	// reload resources
	for (uint i = 0; i < m_resources.size(); ++i)
	{
		m_resources[i]->create(*this);
	}

	m_bRestoring = false;

	if (context())
		setContext(*context(), true);
}

bool SDeviceD3D::isWindowed() const
{ 
	return _lastPresentParameters.Windowed != 0; 
}

PtrD3D<IDirect3DTexture9> SDeviceD3D::createTexture(int width, int height, bool bRenderTarget)
{
	IDirect3DTexture9* tex = 0;

	if (bRenderTarget)
		_device->CreateTexture(width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, 0);
	else
		_device->CreateTexture(width, height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, 0);

	if (!tex)
		throwf("Couldn't create a new texture");

	return tex;
}

void SDeviceD3D::alpha(bool bUse)
{
	_device->SetRenderState(D3DRS_ALPHABLENDENABLE, bUse);
}

void SDeviceD3D::cull(bool bUse)
{
	if (bUse)
		_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	else
		_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

void SDeviceD3D::viewport(const David::Rect& rect)
{
	IDirect3DSurface9* target;
	_device->GetRenderTarget(0, &target);

	Point2i size;

	if (!target)
		size = D3D().screenSize();
	else
	{
		D3DSURFACE_DESC desc;
		target->GetDesc(&desc);
		size.x = desc.Width;
		size.y = desc.Height;
		target->Release();
	}

	m_d3dView.X = (DWORD)(size.x * rect.min().x);
	m_d3dView.Y = (DWORD)(size.y * rect.min().y);
	m_d3dView.Width = (DWORD)(size.x * (rect.max().x - rect.min().x));
	m_d3dView.Height = (DWORD)(size.y * (rect.max().y - rect.min().y));
	m_d3dView.MinZ = 0;
	m_d3dView.MaxZ = 1;

	_device->SetViewport(&m_d3dView);

	m_viewMin = rect.min();
	m_viewMax = rect.max();
	m_viewMinScreen = Point2i(m_d3dView.X, m_d3dView.Y);
	m_viewMaxScreen = Point2i(m_d3dView.X + m_d3dView.Width, m_d3dView.Y + m_d3dView.Height);

	m_lastRenderTarget = target;
}

David::Rect SDeviceD3D::viewport() const
{
	return David::Rect(m_viewMin, m_viewMax);
}

void SDeviceD3D::addResource(class ID3DResource* p)
{
	assert(!m_resources.contains(p));
	p->create(*this);
	m_resources.add(p);
}

void SDeviceD3D::removeResource(class ID3DResource* p)
{
	p->release(*this);
	m_resources.remove(p);
#if _DEBUG
	// hack for faulty "contains" check (doesn't check removed flag)
	m_resources.flush();
#endif
}

const Point2i& SDeviceD3D::screenSize() const
{
	if (isWindowed())
		return m_winSize;
	else
		return m_screenSize;
}

void SDeviceD3D::setContext(class RenderContext& context, bool forceApply)
{
	if (&context != m_context || forceApply)
	{
		m_context = &context;
		m_context->apply();
	}
}

bool SDeviceD3D::beginScene()
{
	return _device->BeginScene() == D3D_OK;
}

void SDeviceD3D::endScene()
{
	_device->EndScene();
}

void SDeviceD3D::xformPixel()
{
	// ui projection matrix
	Matrix4 m(Matrix4::IDENTITY);
	m(0, 0) = (2.0f / (float)screenSize().x);
	m(1, 1) = (-2.0f / (float)screenSize().y);
	m(3, 0) = -1.0f + (0.5f / (float)screenSize().x) * 0.5f;
	m(3, 1) = 1.0f + (0.5f / (float)screenSize().y) * 0.5f;

	setProjection(m);
	setView(Matrix4::IDENTITY);
}

void SDeviceD3D::setProjection(const Matrix4& m)
{
	_device->SetTransform(D3DTS_PROJECTION, m);
}

void SDeviceD3D::setView(const Matrix4& m)
{
	_device->SetTransform(D3DTS_VIEW, m);
}

void SDeviceD3D::reset()
{
	assert(m_defaultContext);
	setContext(*m_defaultContext, true);
}

void SDeviceD3D::clearTextureXForm(const Matrix4& m, int stage)
{
	_device->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
}

void SDeviceD3D::setTextureXForm(const Matrix4& m, int stage)
{
	_device->SetTransform((D3DTRANSFORMSTATETYPE)((int)D3DTS_TEXTURE0 + stage), m);
	_device->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
}

void SDeviceD3D::markDefaultContext()
{
	if (m_defaultContext)
		delete m_defaultContext;

	m_defaultContext = new RenderContext;
}

void SDeviceD3D::scissor(bool enabled)
{
	if (!(m_caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST))
		EXCEPTIONSTREAM(ExceptionVideo(), "No scissor test available.");

	_device->SetRenderState(D3DRS_SCISSORTESTENABLE, enabled);
}

void SDeviceD3D::setScissorRegion(const David::Rect& region)
{
	RECT r(region);
	r.left = (int)ceil(Mapping::linear(region.min().x, -1.0f, 1.0f, 0.0f, (float)screenSize().x));
	r.right = (int)ceil(Mapping::linear(region.max().x, -1.0f, 1.0f, 0.0f, (float)screenSize().x));
	r.top = (int)ceil(Mapping::linear(region.min().y, 1.0f, -1.0f, 0.0f, (float)screenSize().y));
	r.bottom = (int)ceil(Mapping::linear(region.max().y, 1.0f, -1.0f, 0.0f, (float)screenSize().y));

	_device->SetScissorRect(&r);
}

void SDeviceD3D::lighting(bool enabled)
{
	D3DD().SetRenderState(D3DRS_LIGHTING, enabled);
}

PtrD3D<IDirect3DTexture9> SDeviceD3D::nullTexture()
{
	if (!_nullTexture)
	{
		assert(_device);
		_nullTexture = createTexture(1, 1, false);

		D3DLOCKED_RECT rect;
		DX_ENSURE(_nullTexture->LockRect(0, &rect, 0, 0));

		memset(rect.pBits, 0, 4);

		_nullTexture->UnlockRect(0);
	}

	return _nullTexture;
}
