#ifndef STANDARD_EMBEDDEDPTR_H
#define STANDARD_EMBEDDEDPTR_H

#include "Standard/api.h"
#include "SmartPtr.h"

class STANDARD_API IEmbeddedPtrHost
{
public:
	virtual ~IEmbeddedPtrHost() = 0;

	virtual const class EmbeddedPtrHost* host() const = 0;
	virtual class EmbeddedPtrHost* host() = 0;
};

inline IEmbeddedPtrHost::~IEmbeddedPtrHost()
{
}

class STANDARD_API EmbeddedPtrHost : virtual public IEmbeddedPtrHost
{
public:
	EmbeddedPtrHost();
	EmbeddedPtrHost(const EmbeddedPtrHost& copy);

	virtual ~EmbeddedPtrHost();

	const SmartPtr<EmbeddedPtrHost*>& masterRef() const { return _masterRef; }
	class EmbeddedPtrBase& self() const { return *_self; }

protected:
	virtual const class EmbeddedPtrHost* host() const { return this; }
	virtual class EmbeddedPtrHost* host() { return this; }

	SmartPtr<EmbeddedPtrHost*> _masterRef;
	class EmbeddedPtrBase* _self;
};

class STANDARD_API EmbeddedPtrBase
{
public:
	EmbeddedPtrBase(const EmbeddedPtrBase& ptr) :
		_ptr(ptr._ptr)
	{
	}

	EmbeddedPtrBase(const IEmbeddedPtrHost* ptr)
	{
		if (ptr)
			assign(ptr->host());
	}

protected:
	void assign(const EmbeddedPtrBase& p)
	{
		_ptr = p._ptr;
	}

	void assign(const IEmbeddedPtrHost* p)
	{
		if (p && p->host())
		{
			_ptr = p->host()->masterRef();
		}
		else
		{
			_ptr = 0;
		}
	}

	SmartPtr<EmbeddedPtrHost*> _ptr;

	EmbeddedPtrBase() :
		_ptr(0)
	{}
};

// EmbeddedPtr Class
template <class T, class PtrBaseSuper>
class TEmbeddedPtr : public PtrBaseSuper
{
	using PtrBaseSuper::_ptr;
public:
	// construct/copy
	TEmbeddedPtr() :
		PtrBaseSuper()
#if _DEBUG
		,typedPtr(0)
#endif
	{
	}

	TEmbeddedPtr(const TEmbeddedPtr<T, PtrBaseSuper>& ptr) :
		PtrBaseSuper(ptr)
#if _DEBUG
		,typedPtr(0)
#endif
	{
#if _DEBUG
		typedPtr = (T*)ptr;
#endif
	}

	TEmbeddedPtr(const T* ptr) :
		PtrBaseSuper(ptr)
#if _DEBUG
		,typedPtr(0)
#endif
	{
#if _DEBUG
		typedPtr = (T*)ptr;
#endif
	}

	TEmbeddedPtr(const PtrBaseSuper& ptr) :
		PtrBaseSuper(ptr)
#if _DEBUG
		,typedPtr(0)
#endif
	{
#if _DEBUG
		if (_ptr && *_ptr)
			typedPtr = (T*)*_ptr;
		else
			typedPtr = 0;
#endif
	}

	TEmbeddedPtr<T, PtrBaseSuper>& operator = (const TEmbeddedPtr<T, PtrBaseSuper>& p)
	{
		PtrBaseSuper::assign(p);
#if _DEBUG
		typedPtr = (T*)p;
#endif
		return *this;
	}

	// cast
	T& operator*() const { return *(T*)*_ptr; }
	T* operator ->() const { if (_ptr && *_ptr) return (T*)*_ptr; else return 0; }

	// It's okay to cast EmbeddedPtrs implicitly to regular pointers, it's just inefficient to cast back and
	// forth a lot.
	operator T*() const { if (_ptr && *_ptr) return (T*)*_ptr; else return 0; }

	// cast to any other EmbeddedPtr types
	template <class PT>
	operator TEmbeddedPtr<PT, PtrBaseSuper>() const
	{
		PT* c = ptr(); // compile error here if conversion is unsafe, use downcast function if necessary
		TEmbeddedPtr<PT, PtrBaseSuper> newPtr((PtrBaseSuper&)*this);
#if _DEBUG
		newPtr.typedPtr = c;
#endif
		return newPtr;
	}

	// downcast
	template<class AnyT>
	TEmbeddedPtr<AnyT, PtrBaseSuper> downcast() const
	{
		return TEmbeddedPtr<AnyT, PtrBaseSuper>((AnyT*)ptr());
	}

	// query
	T* ptr() const
	{
		if (_ptr && *_ptr)
			return (T*)*_ptr;
		else
			return 0;
	}

#if _DEBUG
	// debugging, so we can see our type
	T* typedPtr;
#endif

protected:
	// template function - so a smart pointer can be cast to a smart pointer of base type
	template<class PT>
	void assign(const TEmbeddedPtr<PT, PtrBaseSuper>& p)
	{
		T* c = p.ptr(); // compile error here if conversion is unsafe, use EmbeddedPtr upcast function if necessary

		PtrBaseSuper::assign(p);
#if _DEBUG
		typedPtr = (T*)c;
#endif
	}
};

template <class T>
class EmbeddedPtr : public TEmbeddedPtr<T, EmbeddedPtrBase>
{
public:
	typedef TEmbeddedPtr<T, EmbeddedPtrBase> Super;

	// construct/copy
	EmbeddedPtr()
	{
	}

	EmbeddedPtr(const EmbeddedPtr& ptr) :
		Super(ptr)
	{
	}

	EmbeddedPtr(const T* ptr) :
		Super(ptr)
	{
	}

	EmbeddedPtr& operator = (const EmbeddedPtr& p)
	{
		Super::operator=(p);
		return *this;
	}

	template<class AnyT>
	EmbeddedPtr<AnyT> downcast() const
	{
		return EmbeddedPtr<AnyT>((AnyT*)this->ptr());
	}

	template <class PT>
	operator EmbeddedPtr<PT>() const
	{
		PT* c = this->ptr(); // compile error here if conversion is unsafe, use downcast function if necessary
		return EmbeddedPtr<PT>(c);
	}
};

#if IS_MSVC
	template <class T>
	size_t hash_value(const EmbeddedPtr<T>& key)
	{
		return stdext::hash_value(key.ptr());
	}
#elif IS_GNUC
	#include <unordered_map>

	namespace std
	{
		template <class T>
		struct hash<EmbeddedPtr<T> > : public std::unary_function<EmbeddedPtr<T>, size_t>
		{
			size_t operator()(const EmbeddedPtr<T>& input) const
			{
				return hash<void*>().operator()(input.ptr());
			}
		};
	}
#else
	#error "Undefined compiler."
#endif

#endif
