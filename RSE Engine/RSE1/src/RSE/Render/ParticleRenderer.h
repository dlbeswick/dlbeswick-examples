// ------------------------------------------------------------------------------------------------
//
// ParticleRenderer
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "Game/Object.h"
class VertexBufferStream;
class TextureAnimationFrame;

class ParticleRenderer : public Object
{
public:
	virtual void draw();
	virtual void update(float delta);

	ParticleRenderer();
	~ParticleRenderer();

	VertexBufferStream& streamFor(const TextureAnimationFrame* f);

protected:
	struct FVF
	{
		float pos[3];
		D3DCOLOR c;
		float tex[2];
	};

	typedef stdext::hash_map<const TextureAnimationFrame*, VertexBufferStream*> BufferMap;

	BufferMap m_buffers;
	uint m_bufferSize;
	DWORD m_fvf;
};