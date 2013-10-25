#include "Standard/pch.h"
#include "CriticalSectionBlock.h"

CriticalSectionBlock::CriticalSectionBlock(CriticalSection& obj, bool tryOnly) :
	m_obj(obj)
{
	if (!tryOnly)
		m_obj.enter();
	else
		m_obj.tryEnter();
}

CriticalSectionBlock::CriticalSectionBlock(const CriticalSection& obj, bool tryOnly) :
	m_obj(const_cast<CriticalSection&>(obj))
{
	if (!tryOnly)
		m_obj.enter();
	else
		m_obj.tryEnter();
}

CriticalSectionBlock::~CriticalSectionBlock()
{
	m_obj.leave();
}
