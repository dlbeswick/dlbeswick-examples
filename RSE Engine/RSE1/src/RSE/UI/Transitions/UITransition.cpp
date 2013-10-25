// ------------------------------------------------------------------------------------------------
//
// UITransition
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "UITransition.h"
#include "TransitionManager.h"
#include "Exception/Video.h"
#include "Render/Camera.h"
#include "Render/D3DPainter.h"
#include "Render/SDeviceD3D.h"
#include "UI/Dialog.h"
#include "UI/DialogMgr.h"

IMPLEMENT_RTTI(UITransition);
IMPLEMENT_RTTI(TransitionRenderToTexture);
IMPLEMENT_RTTI(TransitionEffect);

UITransition::UITransition(TransitionEffect* src, TransitionEffect* dst, bool simultaneous, bool destroy) :
	m_src(src),
	m_dst(dst),
	m_simultaneous(simultaneous)
{
	Transitions.add(this);

	if (m_src)
	{
		m_src->state = TransitionEffect::TRANSITION_OUT;
		m_src->destroyOnFinish = destroy;
		m_src->init();
		m_src->m_src->setDrawOwner(this);
		m_src->m_owner = this;
	}

	if (m_dst)
	{
		m_dst->state = TransitionEffect::TRANSITION_IN;
		m_dst->destroyOnFinish = false;
		m_dst->init();
		m_dst->m_src->setDrawOwner(this);
		m_dst->m_owner = this;
	}
}

UITransition::~UITransition()
{
	delete m_src;
	delete m_dst;
}

void UITransition::update(float delta)
{
	Super::update(delta);

	if (m_src && m_src->finished())
	{
		delete m_src;
		m_src = 0;
	}

	if (m_dst && m_dst->finished())
	{
		delete m_dst;
		m_dst = 0;
	}

	if (!m_src && !m_dst)
	{
		self().destroy();
		return;
	}

	if (m_src)
	{
		m_src->update(delta);

		if (!m_simultaneous)
			return;
	}

	if (m_dst)
		m_dst->update(delta);
}

void UITransition::draw()
{
	if (m_dst && !m_dst->finished() && (m_simultaneous || !m_src))
		m_dst->draw();

	if (m_src && !m_src->finished())
		m_src->draw();
}

UIElement* UITransition::srcElement() const 
{ 
	if (!m_src)
		return 0;

	return m_src->m_owner; 
}

UIElement* UITransition::dstElement() const 
{ 
	if (!m_dst)
		return 0;

	return m_dst->m_owner; 
}

// TransitionEffect
TransitionEffect::TransitionEffect(const PtrGC<UIElement>& src, float time) :
	m_src(src),
	m_time(time),
	m_elapsed(0)
{
}

TransitionEffect::~TransitionEffect()
{
	if (m_src)
	{
		m_src->setInputEnabled(m_oldInputEnabled);

		if (destroyOnFinish)
		{
			m_src.destroy();
		}
		else
		{
			m_src->setDrawOwner(0);
		}
	}
}

void TransitionEffect::init()
{
	m_oldInputEnabled = m_src->inputEnabled();

	if (state == TRANSITION_OUT)
	{
		m_src->setInputEnabled(false);
	}
}

void TransitionEffect::update(float delta)
{
	Super::update(delta);
	m_src->update(delta);
	m_elapsed += delta;
}

float TransitionEffect::alpha()
{
	if (state == TRANSITION_IN)
		return clamp(m_elapsed / m_time);
	else
		return clamp(1.0f - (m_elapsed / m_time));
}

bool TransitionEffect::finished() 
{ 
	return !m_src || m_elapsed > m_time; 
}

// TransitionRenderToTexture
TransitionRenderToTexture::TransitionRenderToTexture(const PtrGC<UIElement>& src, float time, int texSize) :
	Super(src, time),
	m_firstDraw(true),
	m_tex(0)
{
	IDirect3DTexture9* tex;
	D3DD().CreateTexture(texSize, texSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, 0);
	m_tex = tex;

	IDirect3DSurface9* surf;
	D3DD().GetDepthStencilSurface(&surf);
	
	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	D3DD().CreateDepthStencilSurface(texSize, texSize, desc.Format, D3DMULTISAMPLE_NONE, 0, true, &surf, 0);
	m_depth = surf;

	if (!m_tex)
		throwf("TransitionRenderToTexture couldn't make a texture.");
}

TransitionRenderToTexture::~TransitionRenderToTexture()
{
}

void TransitionRenderToTexture::draw()
{
	if (finished())
		return;

	Super::draw();

	// render src element to texture
	if (needsPerFrameRenders() || m_firstDraw)
	{
		IDirect3DSurface9 *pOldTarget;
		DX_ENSURE(D3DD().GetRenderTarget(0, &pOldTarget));

		IDirect3DSurface9 *pOldDepth = 0;
		D3DD().GetDepthStencilSurface(&pOldDepth);
		D3DD().SetDepthStencilSurface(m_depth.ptr());

		// Render UI element to texture
		IDirect3DSurface9 *pTexSurface;
		DX_ENSURE(m_tex->GetSurfaceLevel(0, &pTexSurface));
		DX_ENSURE(D3DD().SetRenderTarget(0, pTexSurface));

		m_src->drawByOwner(m_owner);
	
		if (pOldTarget)
		{
			D3DD().SetRenderTarget(0, pOldTarget);
			pOldTarget->Release();
		}
		if (pOldDepth)
		{
			D3DD().SetDepthStencilSurface(pOldDepth);
			pOldDepth->Release();
		}
		if (pTexSurface)
			pTexSurface->Release();
	}
	
	m_firstDraw = false;
}

void TransitionRenderToTexture::init()
{
	Super::init();
}

