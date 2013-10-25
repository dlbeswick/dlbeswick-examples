// ------------------------------------------------------------------------------------------------
//
// Base
// Base class for many RSE engine objects, contains streaming functionality, 
// counter functionality, config file support.
//
// Standard streamable functionality with polymorphic restoration.
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_BASE_H
#define STANDARD_BASE_H

#include "Standard/api.h"
#include "Config.h"
#include "Streamable.h"
#include "Counter.h"
#include "Registrar.h"
#include "SmartPtr.h"

class STANDARD_API Base : public Streamable, public Registrar<Base>
{
	USE_RTTI(Base, Base);
	USE_COUNTERS(Base);

public:
	Base();

	virtual ~Base();

	template <class T> T& cast()
	{ 
		if (rtti().isA(T::static_rtti()))
			return *(T*)this;
		else
			EXCEPTIONSTREAM(ExceptionRTTI(), "Cannot cast from '" << rtti().className() << "' to '" << T::static_rtti().className());
	}

	template <class T> T& cast() const
	{ 
		return const_cast<Base*>(this)->cast<T>();
	}

	virtual void construct();

	// construction
	static Base* newObject(const RTTI& rtti, const std::string& configOverride = std::string());
	static Base* newObject(const std::string& className, const std::string& configOverride = std::string());

	template <class Arg1>
	static Base* newObjectArg1(const RTTI& rtti, const Arg1& arg1, const std::string& configOverride = std::string())
	{
		return Registrar<Base>::newObject(rtti, arg1);
	}

	// ini settings
	virtual bool hasConfig() const;
	virtual Config& config();
	virtual const Config& config() const;

	virtual void clearConfig();
	// Sets the target of calls to 'get' and 'set'.
	// If 'category' is given, then that category is supplied to config.get.
	// If category is not supplied or a value is not found for that category, then the 
	// RTTI class name of the object is used as the category for 'get' operations.
	virtual void setConfig(Config& config, const std::string& category = std::string());
	
	virtual const std::string& configCategory() const;// rtti name is used if null
	virtual void setConfigCategory(const std::string& category);

	void streamFromConfig();

	static Base* streamFromConfig(Config& config, const std::string& category);

	virtual void configLoad() {};  // override this to perform ini file loading, and call it after construction.

	template <class T> T get(const char* name, const T& defaultVal = T())
	{
		if (!m_currentConfig)
			throwf("No current config defined for " + rtti().className());
		if (&rtti() == &Base::static_rtti())
			throwf("'get' called for Base type -- are you calling get in a constructor?");
		return m_currentConfig->get<T>(rtti(), name, m_configCategoryOverride, defaultVal);
	}

	template <class T> void get(const char* name, T& var)
	{
		if (!m_currentConfig)
			throwf("No current config defined for " + rtti().className());
		if (&rtti() == &Base::static_rtti())
			throwf("'get' called for Base type -- are you calling get in a constructor?");
		m_currentConfig->get<T>(rtti(), name, m_configCategoryOverride, var);
	}

	template <class T> void set(const char* name, const T& var)
	{
		if (!m_currentConfig)
			throwf("No current config defined for " + rtti().className());
		if (&rtti() == &Base::static_rtti())
			throwf("'set' called for Base type -- are you calling set in a constructor?");

		if (!m_configCategoryOverride.empty())
			m_currentConfig->set<T>(m_configCategoryOverride, name, var);
		else if (&rtti() != &Base::static_rtti()) // if object is destroyed before construction, "set"s in destructors can write erroneously to Base category
			m_currentConfig->set<T>(rtti().className(), name, var);
	}

	// update
	virtual void update(float delta);

protected:
	// streaming overrides
	virtual void streamVars(StreamVars& v);

	// clients should override this method in derived class if they wish to perform the post-construction
	// call of "construct" themselves, perhaps to suit a different construction scheme.
	virtual bool needPostNewObjectConstructCall() const;

#if !NDEBUG
	bool _constructed;
#endif

private:
	void writeRTTI(otextstream& s) const;
	static Base* readRTTI(itextstream& s);

	void writeRTTI(obinstream& s) const;
	static Base* readRTTI(ibinstream& s);

	template <class T> friend itextstream& operator >> (itextstream& s, T*& rhs);
	friend otextstream& operator << (otextstream& s, const Base* rhs);
	template <class T> friend ibinstream& operator >> (ibinstream& s, T*& rhs);
	friend obinstream& operator << (obinstream& s, const Base* rhs);

	template <class T> friend itextstream& operator >> (itextstream& s, SmartPtr<T>& rhs);
	template <class T> friend otextstream& operator << (otextstream& s, const SmartPtr<T>& rhs);
	template <class T> friend ibinstream& operator >> (ibinstream& s, SmartPtr<T>& rhs);
	template <class T> friend obinstream& operator << (obinstream& s, const SmartPtr<T>& rhs);

	Config* m_currentConfig;
	std::string m_configCategoryOverride;
};

// streaming operators
// read/write to pointers to dynamically create objects of rtti type and stream values in
template <class T> inline itextstream& operator >> (itextstream& s, T*& rhs) { rhs = (T*)Base::readRTTI(s); return s; }
inline otextstream& operator << (otextstream& s, const Base* rhs) { rhs->writeRTTI(s); return s; }
template <class T> inline ibinstream& operator >> (ibinstream& s, T*& rhs) { rhs = (T*)Base::readRTTI(s); return s; }
inline obinstream& operator << (obinstream& s, const Base* rhs) { rhs->writeRTTI(s); return s; }

template <class T> inline itextstream& operator >> (itextstream& s, SmartPtr<T>& rhs) { rhs = (T*)Base::readRTTI(s); return s; }
template <class T> inline otextstream& operator << (otextstream& s, const SmartPtr<T>& rhs) { rhs->writeRTTI(s); return s; }
template <class T> inline ibinstream& operator >> (ibinstream& s, SmartPtr<T>& rhs) { rhs = (T*)Base::readRTTI(s); return s; }
template <class T> inline obinstream& operator << (obinstream& s, const SmartPtr<T>& rhs) { rhs->writeRTTI(s); return s; }

#endif
