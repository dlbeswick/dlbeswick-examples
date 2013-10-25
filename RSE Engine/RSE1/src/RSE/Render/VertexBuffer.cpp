// ------------------------------------------------------------------------------------------------
//
// VertexBuffer
//
// ------------------------------------------------------------------------------------------------
#include "pch.h"
#include "VertexBuffer.h"
#include "SDeviceD3D.h"
#include "Standard/Exception.h"
#include "Standard/STLHelp.h"

VertexBuffer::VertexBuffer(uint vertSize, DWORD fvf, uint numVerts, DWORD usage, D3DPOOL pool, DWORD flags)
{
	m_vertSize = vertSize;
	m_usage = usage;
	m_size = numVerts * m_vertSize;
	m_pool = pool;
	m_fvf = fvf;
	m_flags = flags;
	m_ptr = 0;

	m_buffer = 0;
	D3D().addResource(this);
}

void VertexBuffer::createBuffer()
{
	if (D3DD().CreateVertexBuffer(m_size, m_usage, m_fvf, m_pool, &m_buffer, 0) != D3D_OK)
		throwf("VertexBuffer: create failed.");
}

VertexBuffer::~VertexBuffer()
{
	D3D().removeResource(this);
}

void VertexBuffer::insert(void* data, uint verts)
{
	uint dataSize = verts * m_vertSize;

	assert(m_insertPtr + dataSize <= m_ptrEnd);

	memcpy(m_insertPtr, data, dataSize);
	m_insertPtr += dataSize;
	m_used = m_insertPtr - m_ptr;
}

uint VertexBuffer::load(uint verts)
{
	uint dataSize = verts * m_vertSize;

	assert(!m_ptr);

	if (m_buffer->Lock(0, dataSize, (void**)&m_ptr, m_flags) == D3D_OK)
		m_ptrEnd = m_ptr + std::min(dataSize, m_size / m_vertSize);
	else
	{
		m_ptr = 0;
		m_ptrEnd = 0;
	}

	m_insertPtr = m_ptr;
	m_used = 0;

	return m_ptrEnd - m_ptr;
}

void VertexBuffer::finish()
{
	if (m_ptr)
		m_buffer->Unlock();

	m_used = (m_insertPtr - m_ptr) / m_vertSize;
	m_ptr = 0;
	m_insertPtr = 0;
}

void VertexBuffer::release(SDeviceD3D& device)
{
	finish();
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = 0;
	}
}

void VertexBuffer::create(SDeviceD3D& device)
{
	createBuffer();
}

//////////////

VertexBufferStream::VertexBufferStream(uint vertSize, DWORD fvf, uint granularityBytes, DWORD usage, D3DPOOL pool, DWORD flags)
{
	m_vertSize = vertSize;
	m_usage = usage;
	m_granularity = granularityBytes;
	m_pool = pool;
	m_fvf = fvf;
	m_flags = flags;
	m_ptr = 0;
	m_ptrEnd = 0;
	m_current = m_renderEnd = m_buffers.end();
}

VertexBufferStream::~VertexBufferStream()
{
	for (Buffers::iterator i = m_buffers.begin(); i != m_buffers.end(); ++i)
		delete i->buffer;
}

uint VertexBufferStream::loadChunk()
{
	if (m_current != m_buffers.end())
	{
		if (m_current->bLocked)
		{
			m_current->buffer->buffer()->Unlock();
			m_current->bLocked = false;
			m_ptr = 0;
			++m_current;
		}
	}
	
	if (m_current == m_buffers.end())
	{
		// if no more buffers available
		createBuffer();
		m_current = --m_buffers.end();
	}

	if (!m_ptr)
	{
		m_current->buffer->buffer()->Lock(0, 0, (void**)&m_ptr, m_flags);
		m_current->bLocked = true;
		m_ptrEnd = m_ptr + m_granularity;
	}

	return m_granularity;
}

void VertexBufferStream::finish()
{
	if (m_current != m_buffers.end())
	{
		if (m_current->bLocked)
		{
			m_current->buffer->buffer()->Unlock();
			m_current->bLocked = false;
			m_current->finalUtilised = m_granularity - (m_ptrEnd - m_ptr);

			m_renderEnd = m_current;
			++m_renderEnd;
		}
		else
		{
			m_current->finalUtilised = 0;
			m_renderEnd = m_buffers.begin();
		}
	}

	m_ptr = 0;
	m_ptrEnd = 0;

	m_current = m_buffers.begin();
}

void VertexBufferStream::createBuffer()
{
	VertexBuffer* buffer = 0;
	buffer = new VertexBuffer(m_vertSize, m_fvf, m_granularity / m_vertSize, m_usage, m_pool);

	m_buffers.push_back(BufferData());
	m_buffers.back().buffer = buffer;
	m_buffers.back().bLocked = false;
}

void VertexBufferStream::insert(void* data, uint numVerts)
{
	uint dataSize = numVerts * m_vertSize;
	
	ensure(numVerts);
	
	memcpy(ptrAccess(), (char*)data, dataSize);
	m_ptr += dataSize;
}

void VertexBufferStream::ensure(uint numVerts)
{
	uint available = ptrEnd() - ptrAccess();
	if (available < numVerts * m_vertSize)
		loadChunk();
}

void VertexBufferStream::drawTriList()
{
	finish();

	for (VertexBufferStream::Buffers::const_iterator i = begin(); i != renderEnd(); ++i)
	{
		assert((i->finalUtilised / m_vertSize) % 3 == 0);

		D3DD().SetStreamSource(0, i->buffer->buffer(), 0, m_vertSize);
		D3DD().DrawPrimitive(D3DPT_TRIANGLELIST, 0, i->finalUtilised / m_vertSize / 3);
	}
}