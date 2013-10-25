// ------------------------------------------------------------------------------------------------
//
// ParticleRenderer
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "ParticleRenderer.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleSystemInline.h"
#include "Render/SDeviceD3D.h"
#include "Render/TextureAnimator.h"
#include "Render/TextureAnimationTypes.h"
#include "Standard/Profiler.h"

ParticleRenderer::ParticleRenderer()
{
	m_fvf = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_DIFFUSE;
	m_bufferSize = 1024 * 12 * sizeof(FVF); // 1024 particles of 12 verts each = 12288 verts
}

ParticleRenderer::~ParticleRenderer()
{
	for (BufferMap::iterator i = m_buffers.begin(); i != m_buffers.end(); ++i)
	{
		delete i->second;
	}
}

#pragma inline_depth(255)

void ParticleRenderer::update(float delta)
{
	Profile("Particles Update");

	BufferMap::iterator b;

	m_children.flush();
	for (ObjectList::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		ParticleSystem& s = *(ParticleSystem*)i->ptr();
		
		s.inlineUpdate();

		for (ObjectList::const_iterator j = s.children().begin(); j < s.children().end(); ++j)
		{
			ParticleEmitter* e = Cast<ParticleEmitter>(j->ptr());

			e->inlineUpdate(delta);
		}

		if (s.children().size() == 0 && !s.bNeverDestroy)
		{
			i->destroy();
		}
	}
}

#pragma inline_depth(255)

void ParticleRenderer::draw()
{
	D3D().zbuffer(false);
	D3DD().SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	D3DD().SetRenderState(D3DRS_LIGHTING, false);
	D3DD().SetRenderState(D3DRS_COLORVERTEX, true);
	D3DD().SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	D3DD().SetFVF(m_fvf);

	int num = 0;

	for (BufferMap::iterator i = m_buffers.begin(); i != m_buffers.end(); ++i)
	{
		D3DD().SetTexture(0, i->first->texture.ptr());

		VertexBufferStream& stream = *i->second;
		stream.finish();

		for (VertexBufferStream::Buffers::const_iterator j = stream.begin(); j != stream.renderEnd(); ++j)
		{
			++num;

			D3DD().SetStreamSource(0, j->buffer->buffer(), 0, sizeof(FVF));
			D3DD().DrawPrimitive(D3DPT_TRIANGLELIST, 0, j->finalUtilised / sizeof(FVF) / 3);
		}
	}

	ProfileValue("PS Vert Buf Bytes", num * m_bufferSize);
}

VertexBufferStream& ParticleRenderer::streamFor(const TextureAnimationFrame* f)
{
	assert(f);
	assert(f->texture);

	BufferMap::iterator b = m_buffers.find(f);
	if (b == m_buffers.end())
		b = m_buffers.insert(BufferMap::value_type(f, new VertexBufferStream(sizeof(FVF), m_fvf, m_bufferSize))).first;

	return *b->second;
}
