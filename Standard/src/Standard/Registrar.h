// ------------------------------------------------------------------------------------------------
//
// Registrar
// Makes a global list of RTTI objects.
//
// ------------------------------------------------------------------------------------------------
#ifndef STANDARD_REGISTRAR_H
#define STANDARD_REGISTRAR_H

#include "Standard/api.h"
#include "Log.h"
#include "RTTI.h"
#include "STLHelp.h"

class RTTI;

class STANDARD_API RegistrarBase
{
public:
	class ConstructorBase
	{
	public:
		virtual RegistrarBase* construct() { return 0; }
		virtual RegistrarBase* construct(void* arg) { return 0; }
		virtual RegistrarBase* copy(const RegistrarBase& t) { return 0; }
	};

	template <class T>
	class CopyConstructor : public ConstructorBase
	{
	public:
		virtual RegistrarBase* copy(const RegistrarBase& t) { RegistrarBase* newObj = new T((const T&)t); return newObj; }
	};

	template <class T>
	class Constructor : public CopyConstructor<T>
	{
	public:
		virtual RegistrarBase* construct()
		{
			return new T;
		}
	};

	template <class T, class Arg1>
	class ConstructorArg1 : public CopyConstructor<T>
	{
	public:
		virtual RegistrarBase* construct(void* arg)
		{
			return new T(*(Arg1*)arg);
		}
	};

	virtual bool needPostNewObjectConstructCall() const { return true; }
	virtual void construct() {};
};


template <class T>
class Registrar : public RegistrarBase
{
public:
	virtual ~Registrar() {};

	struct Registrant
	{
		Registrant(const RTTI& _rtti, typename RegistrarBase::ConstructorBase* _func, const std::string& _name) :
			rtti(_rtti), func(_func), name(_name)
		{
		}

		~Registrant()
		{
			delete func;
		}

		const RTTI& rtti;
		std::string name;
		typename RegistrarBase::ConstructorBase* func;
	};

	typedef std::vector<const Registrant*> RegistrantsList;
	typedef stdext::hash_map<std::string, Registrant*> RegistrantsName;
	typedef stdext::hash_map<const RTTI*, Registrant*> RegistrantsRTTI;

	static const Registrant& registerObject(const RTTI& rtti, ConstructorBase* func, const std::string& name)
	{
		assert(regName().find(name) == regName().end());
		assert(regRTTI().find(&rtti) == regRTTI().end());

		Registrant* r = new Registrant(rtti, func, name);

		regName()[name] = r;
		regRTTI()[&rtti] = r;
		regRTTIName()[rtti.className()] = r;

		return *r;
	}

	static const Registrant* registrant(const RTTI& rtti)
	{
		const RegistrantsRTTI& m = regRTTI();
		typename RegistrantsRTTI::const_iterator i = m.find(&rtti);
		if (i == m.end())
			return 0;

		return i->second;
	}

	static const Registrant* registrant(const std::string& className)
	{
		const RegistrantsName& m = regRTTIName();
		typename RegistrantsName::const_iterator i = m.find(className);
		if (i == m.end())
			return 0;

		return i->second;
	}

	static const Registrant* registrantByDesc(const std::string& desc)
	{
		const RegistrantsName& m = regName();
		typename RegistrantsName::const_iterator i = m.find(desc);
		if (i == m.end())
			return 0;

		return i->second;
	}

	static T* newObject(const RTTI& rtti)
	{
		const Registrant* r = registrant(rtti);
		if (!r)
			return 0;

		RegistrarBase* newObj = r->func->construct();

		if (newObj && newObj->needPostNewObjectConstructCall())
			newObj->construct();

		return (T*)newObj;
	}

	template <class Arg1>
	static T* newObject(const RTTI& rtti, Arg1 arg1)
	{
		const Registrant* r = registrant(rtti);
		if (!r)
			return 0;

		RegistrarBase* newObj = r->func->construct((void*)&arg1);

		if (newObj && newObj->needPostNewObjectConstructCall())
			newObj->construct();

		return (T*)newObj;
	}

	static T* newObject(const std::string& className)
	{
		const Registrant* r = registrant(className);
		if (!r)
			return 0;

		RegistrarBase* newObj = r->func->construct();

		if (newObj && newObj->needPostNewObjectConstructCall())
			newObj->construct();

		return (T*)newObj;
	}

	static T* newObjectByDesc(const std::string& desc)
	{
		const Registrant* r = registrantByDesc(desc);
		if (!r)
			return 0;

		RegistrarBase* newObj = r->func->construct();

		if (newObj && newObj->needPostNewObjectConstructCall())
			newObj->construct();

		return (T*)newObj;
	}

	template <class U>
	U* clone()
	{
		const Registrant* r = registrant(((U*)this)->rtti());
		U* newObj = (U*)r->func->copy(*this);

		return newObj;
	}

	static const RegistrantsName& registrants()
	{
		return regName();
	}

	static void registrantsOfClass(const RTTI& rtti, RegistrantsList& l)
	{
		RegistrantsRTTI& r = regRTTI();
		for (typename RegistrantsRTTI::iterator i = r.begin(); i != r.end(); ++i)
		{
			if (i->second->rtti.isA(rtti))
				l.push_back(i->second);
		}
	}

	static void registrantNamesOfClass(const RTTI& rtti, std::vector<std::string>& l)
	{
		RegistrantsList r;
		registrantsOfClass(rtti, r);
		for (uint i = 0; i < r.size(); ++i)
			l.push_back(r[i]->rtti.className());
	}

private:
	class Registrants
	{
	public:
		Registrants()
		{
		}

		RegistrantsName name;
		RegistrantsName rttiName;
		RegistrantsRTTI rtti;
	};

	static Registrants& reg()
	{
		static Registrants r;
		return r;
	}

	static RegistrantsName& regName()
	{
		return reg().name;
	}

	static RegistrantsRTTI& regRTTI()
	{
		return reg().rtti;
	}

	static RegistrantsName& regRTTIName()
	{
		return reg().rttiName;
	}
};

#define REGISTER_RTTI(rttiClass) \
	IMPLEMENT_RTTI(rttiClass); \
	rttiClass::Registrant registrant_entry_##rttiClass = \
		rttiClass::registerObject(rttiClass::static_rtti(), new RegistrarBase::Constructor<rttiClass>, #rttiClass);

#define REGISTER_RTTI_NAME(rttiClass, name) \
	IMPLEMENT_RTTI(rttiClass); \
	rttiClass::Registrant registrant_entry_##rttiClass = \
		rttiClass::registerObject(rttiClass::static_rtti(), new RegistrarBase::Constructor<rttiClass>, name);

#define REGISTER_RTTI_ARG1(rttiClass,arg1Class) \
	IMPLEMENT_RTTI(rttiClass); \
	rttiClass::Registrant registrant_entry_##rttiClass = \
		rttiClass::registerObject(rttiClass::static_rtti(), new RegistrarBase::ConstructorArg1<rttiClass, arg1Class >, #rttiClass);

#endif

