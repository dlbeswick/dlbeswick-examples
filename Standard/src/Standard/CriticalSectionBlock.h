#ifndef STANDARD_CRITICALSECTIONBLOCK_H
#define STANDARD_CRITICALSECTIONBLOCK_H

#include "Standard/api.h"
#include "Standard/CriticalSection.h"

class STANDARD_API CriticalSectionBlock
{
public:
	CriticalSectionBlock(CriticalSection& obj, bool tryOnly = false);

	CriticalSectionBlock(const CriticalSection& obj, bool tryOnly = false);

	~CriticalSectionBlock();

protected:
	CriticalSection& m_obj;
};

#define Critical(x) CriticalSectionBlock critical_section(x)
#define TryCritical(x) CriticalSectionBlock critical_section(x, true)

#endif
