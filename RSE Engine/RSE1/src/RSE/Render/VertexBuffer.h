// ------------------------------------------------------------------------------------------------
//
// VertexBuffer
//
// ------------------------------------------------------------------------------------------------
#pragma once


#include "RSE/RSE.h"
#include "RSE/Render/ID3DResource.h"

class RSE_API VertexBuffer : public ID3DResource
{
public:
	VertexBuffer(uint vertSize, DWORD fvf, uint numVerts = 2048, DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DPOOL pool = D3DPOOL_DEFAULT, DWORD flags = D3DLOCK_DISCARD);
	~VertexBuffer();

	operator IDirect3DVertexBuffer9* () const { return m_buffer; }
	IDirect3DVertexBuffer9* buffer() const { return m_buffer; }

	uint size() const { return m_size; }
	uint verts() const { return m_size / m_vertSize; }
	uint vertSize() const { return m_vertSize; }

	void insert(void* data, uint verts);

	// returns number of verts locked
	uint load(uint verts);
	char* ptr() const { return m_ptr; }
	char* current() const { return m_insertPtr; }
	char* end() const { return m_ptrEnd; }
	uint used() const { return m_used; }
	bool full() const { return m_insertPtr >= m_ptrEnd; }
	void finish();

	// ID3DResource
	virtual void release(SDeviceD3D& device);
	virtual void create(SDeviceD3D& device);

protected:
	void createBuffer();

	IDirect3DVertexBuffer9* m_buffer;
	uint m_size;
	uint m_vertSize;
	uint m_used;
	DWORD m_usage;
	D3DPOOL m_pool;
	DWORD m_fvf;
	DWORD m_flags;
	char* m_ptr;
	char* m_ptrEnd;
	char* m_insertPtr;
};

class VertexBufferStream
{
public:
	struct BufferData
	{
		VertexBuffer* buffer;
		bool bLocked;
		uint finalUtilised;
	};

	typedef std::vector<BufferData> Buffers;

	VertexBufferStream(uint vertSize, DWORD fvf, uint granularityBytes = 1024, DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DPOOL pool = D3DPOOL_DEFAULT, DWORD flags = D3DLOCK_DISCARD);
	~VertexBufferStream();

    // iteration
	const Buffers::const_iterator begin() const { return m_buffers.begin(); }
	const Buffers::const_iterator renderEnd() { return m_renderEnd; }

	const Buffers& buffers() const { return m_buffers; }
	uint size() const { return m_buffers.size(); }
	uint sizeBytes() const { return m_buffers.size() * m_granularity; }
	const BufferData& current() const { return *m_current; }

	// call this to insert a number of verts into the buffer
	void insert(void* data, uint numVerts);

	// Call this to insert any data into the buffer. You can stream fvf data instead of writing a struct.
	template <class T>
	void add(const T& data)
	{
		insert((void*)&data, sizeof(data));
	}

	void ensure(uint numVerts);

	// returns amount of data locked
	uint loadChunk();
	char*& ptrAccess() { return m_ptr; }
	char* ptrEnd() const { return m_ptrEnd; }
	void finish();

	void drawTriList();

protected:
	void createBuffer();

	Buffers m_buffers;
	Buffers::iterator m_current;
	Buffers::iterator m_renderEnd;
	uint m_granularity;

	uint m_vertSize;
	DWORD m_usage;
	D3DPOOL m_pool;
	DWORD m_fvf;
	DWORD m_flags;
	char* m_ptr;
	char* m_ptrEnd;
};
