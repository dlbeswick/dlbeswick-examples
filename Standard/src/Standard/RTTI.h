// ------------------------------------------------------------------------------------------------
//
// RTTI
// Put USE_RTTI in header, IMPLEMENT_RTTI in cpp. When declaring the root, let 'base' equal
// 'classname'. Cast with the "Cast" template functions.
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_RTTI_H
#define STANDARD_RTTI_H

#include "Standard/api.h"
#include "Standard/ExceptionStream.h"

class ExceptionRTTI : public ExceptionStream
{};

class RTTI
{
public:
	RTTI(const RTTI* base, const std::string& className) :
	  m_base(base),
	  m_className(className)
	  {}

	const std::string& className() const { return m_className; }
	const RTTI* base() const { return m_base; }
	bool isA(const RTTI& other) const
	{
		const RTTI* testBase = this;
		const RTTI* nextBase;
		do
		{
			if (testBase == &other)
				return true;

			nextBase = testBase->base();
			if (testBase == nextBase)
				return false;

			testBase = testBase->base();
		}
		while (true);

		return false;
	}

	const bool isBase() const { return this == m_base; }

private:
	const RTTI*		m_base;
	std::string		m_className;
};

#define USE_RTTI(classname, base) \
	private: \
	static RTTI class_RTTI; \
	\
	public: \
	typedef base Super; \
	virtual const RTTI& rtti() const { return classname::class_RTTI; } \
	static const RTTI& static_rtti() { return classname::class_RTTI; }

#define IMPLEMENT_RTTI(classname) \
	RTTI classname::class_RTTI(&classname::Super::static_rtti(), #classname);

template <class To, class From>
inline To* Cast(From* what)
{
	if (what && what->rtti().isA(To::static_rtti()))
		return (To*)what;

	return 0;
}

#endif
