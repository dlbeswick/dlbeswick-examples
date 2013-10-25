// ---------------------------------------------------------------------------------------------------------
// 
// SDeviceD3D
// 
// ---------------------------------------------------------------------------------------------------------
#pragma once

#include "RSE/RSE.h"
#include "Standard/Math.h"
#include "Standard/Path.h"
#include "Standard/PendingList.h"
#include "Standard/PtrD3D.h"
#include "Standard/Singleton.h"

class Image;

namespace David
{
	class Rect;
}

class RSE_API SDeviceD3D : public Singleton<SDeviceD3D>
{
public:
	SDeviceD3D();
	~SDeviceD3D();

	void destroy();

	void newDevice(bool bWindowed, int resX = 1024, int resY = 768, int winResX = 640, int winResY = 480, bool b32BitDepthBuffer = false);
	void recreateDevice();

	bool beginScene();
	void endScene();

	void addResource(class ID3DResource* p);
	void removeResource(class ID3DResource* p);

	bool supportsHardwareVP() const;

	bool isDeviceLost();
	bool isWindowed() const;
	void toggleFullscreen();

	const Point2i& screenSize() const;

	IDirect3DDevice9& device() { return *_device; }
	
	class D3DPainter* painter;

	void clearTextureXForm(const Matrix4& m, int stage);

	// context
	/// sets the render context used when 'reset' is called
	void markDefaultContext();
	void setContext(class RenderContext& context, bool forceApply = false);
	class RenderContext* context() const { return m_context; }

	// loading
	PtrD3D<IDirect3DTexture9> loadTexture(const Image& image, bool bMipMap = true, bool makePowerOfTwo = false);
	PtrD3D<IDirect3DTexture9> loadTexture(const Path& fName, bool bMipMap = true, bool makePowerOfTwo = false);
	PtrD3D<IDirect3DTexture9> loadTextureFromResource(const std::string& resName, const char* resType, bool bMipMap = true, bool makePowerOfTwo = false);
	PtrD3D<IDirect3DTexture9> createTexture(int width, int height, bool bRenderTarget = false);

	void alpha(bool bUse);
	void cull(bool bUse);
	void scissor(bool enabled);
	void lighting(bool enabled);
	void texFilter(bool bUseMin, bool bUseMag);
	void viewport(const David::Rect& rect);
	David::Rect viewport() const;
	void wireframe(bool bUse);
	void zbuffer(bool bUse);

	/// Sets the scissor region in viewport coordinates (-1 -> 1).
	void setScissorRegion(const David::Rect& region);

	// Resets all render state to defaults.
	void reset();

	void setProjection(const Matrix4& m);
	void setTextureXForm(const Matrix4& m, int stage);
	void setView(const Matrix4& m);

	// set a traditional pixel-based view and perspective transform.
	void xformPixel();

	const Point2i& viewportMinScreen() const { return m_viewMinScreen; }
	const Point2i& viewportMaxScreen() const { return m_viewMaxScreen; }

	// caps
	int maxHWClipPlanes() const { return m_caps.MaxUserClipPlanes; }

	PtrD3D<IDirect3DTexture9> nullTexture();

protected:
	// fix -- it's for onDeviceLost, but the code that calls it shouldn't be in SAppBase
	friend class SAppBase;

	void newDevice(D3DPRESENT_PARAMETERS& d3dpp, bool isRecreating);
	void onDeviceLost();

	IDirect3D9*			m_pD3D;
	IDirect3DDevice9*	_device;
	D3DPRESENT_PARAMETERS _lastPresentParameters;
	IDirect3DSurface9*	m_lastRenderTarget;
	bool				m_bRestoring;
	bool				_mixedMode;
	Point2i				m_screenSize;
	Point2i				m_winSize;
	Point2				m_viewMin;
	Point2				m_viewMax;
	Point2i				m_viewMinScreen;
	Point2i				m_viewMaxScreen;
	D3DVIEWPORT9		m_d3dView;
	D3DCAPS9			m_caps;
	PtrD3D<IDirect3DTexture9> _nullTexture;

	class RenderContext* m_context;
	class RenderContext* m_defaultContext;

	PendingListNullRemover<class ID3DResource*> m_resources; // list used to re-create resources on lost device
};

inline SDeviceD3D& D3D() { return SDeviceD3D::instance(); }
inline IDirect3DDevice9& D3DD() { return SDeviceD3D::instance().device(); }
