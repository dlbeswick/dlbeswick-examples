#include "dsound/pch.h"
#include "BufferFormat.h"
#include "BufferModifier.h"

////

BufferModifier::~BufferModifier()
{
}

void BufferModifier::setEnabled(bool b) 
{
	m_enabled = b;
}

////

bool BufferModifierClear::modify(short*& buf, int& samples, int stride) 
{ 
	memset(buf, 0, BufferFormat::Short::bytesPerSample() * samples); 
	return true;
}

bool BufferModifierClear::modify(float*& buf, int& samples, int stride) 
{ 
	memset(buf, 0, BufferFormat::Float::bytesPerSample() * samples); 
	return true;
}
