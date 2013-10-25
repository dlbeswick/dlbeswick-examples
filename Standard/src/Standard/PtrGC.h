// ---------------------------------------------------------------------------------------------------------
//
// PtrGC
//
// Pointer wrapper class implementing user-initiated garbage collection.
//
// These pointers have the ability to resolve themselves from old pointers to new on being streamed from disk.
// Perform the "primary save/load" in one place only, then all other save/loads will point to the primary
// streamed data.
// When "destroy" is called on the pointer, all other PtrGCs holding references to the pointer's object are
// nulled.
// "destroy"ed objects are not deleted until PtrGCGlobals::garbage.collect() is called.
//
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_PTRGC_H
#define STANDARD_PTRGC_H

#include "Standard/api.h"
#include "Standard/PtrGCRefCountMethod.h"
#include "Standard/binstream.h"
#include "Standard/textstream.h"

// Dynamic cast convenience
template <class To, class From>
inline PtrGC<To> Cast(const PtrGC<From>& what)
{
	if (what && what->rtti().isA(To::static_rtti()))
		return what.template downcast<To>();

	return 0;
}

// Streaming operators (TODO: make them work)
template <class T>
inline obinstream& operator << (obinstream& s, const PtrGC<T>& obj)
{
	if (obj.ptr())
	{
		// TODO: store ptr in resolver and only write on resolver call, or multiples will be written
		s.s().write((char*)obj.ptr(), sizeof(void*));
		((T*)obj.ptr())->writeRTTI(s);
	}
	else
		s.s() << '0';

	return s;
}

template <class T>
inline ibinstream& operator >> (ibinstream& s, PtrGC<T>& obj)
{
	void* oldPtr = 0;

	if (s.s().peek() == '0')
	{
		obj = 0;
		s.s().seekg(1, std::ios::cur);
		return s;
	}

	s.s().read((char*)oldPtr, sizeof(void*));
	ptrGCManager.resolver->resolve(oldPtr, obj);

	return s;
}

template <class T>
inline otextstream& operator << (otextstream& s, const PtrGC<T>& obj)
{
	if (obj.ptr())
	{
		s << obj.ptr();
		((T*)obj.ptr())->writeRTTI(s);
	}
	else
		s << "0";

	return s;
}

template <class T>
inline itextstream& operator >> (itextstream& s, PtrGC<T>& obj)
{
	void* oldPtr = 0;

	if (s.s().peek() == '0')
	{
		obj = 0;
		s.s().seekg(1, std::ios::cur);
		return s;
	}

	s.s().read((char*)oldPtr, sizeof(void*));
	ptrGCManager.resolver->resolve(oldPtr, obj);

	return s;
}

// msvc hash maps complain about ptrgc unless this is here.
#if IS_MSVC
template <class T>
inline size_t hash_value(const PtrGC<T>& _Keyval)
{	
	return ((size_t)_Keyval.ptr() ^ _HASH_SEED);
}
#elif IS_GNUC
	#include <unordered_map>

	namespace std
	{
		template <class T>
		struct hash<PtrGC<T> > : public std::unary_function<PtrGC<T>, size_t>
		{
			size_t operator()(const PtrGC<T>& input) const
			{
				return hash<void*>().operator()(input.ptr());
			}
		};
	}
#else
	#error "Undefined compiler."
#endif

#endif
