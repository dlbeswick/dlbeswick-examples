#ifndef _STANDARD_STLHELP_H
#define _STANDARD_STLHELP_H

#include "Standard/api.h"
#include "Standard/platform.h"
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <ctype.h>

// sortedKeys
// returns a sorted vector of the keys from a hashmap
template <class T>
inline std::vector<typename T::key_type> sortedKeys(const T& map)
{
	std::vector<typename T::key_type> vec;

	for (typename T::const_iterator i = map.begin(); i != map.end(); ++i)
	{
		vec.push_back(i->first);
	}

	std::sort(vec.begin(), vec.end());

	return vec;
}

// mapValues
// returns a vector of the values from a map
template <class T>
inline std::vector<typename T::mapped_type> mapValues(const T& map)
{
	std::vector<typename T::mapped_type> vec;

	for (typename T::const_iterator i = map.begin(); i != map.end(); ++i)
	{
		vec.push_back(i->second);
	}

	return vec;
}

// tolower
// returns the lower case version of a string
inline std::string strToLower(const std::string& s)
{
	std::string newStr(s);
	std::transform(newStr.begin(), newStr.end(), newStr.begin(), ::tolower);
	return newStr;
}

// Owner pointer list helpers
template<class T> void freeSTL(T &stl)
{
	for (typename T::iterator i = stl.begin(); i != stl.end(); i++)
	{
		delete *i;
	}

	stl.clear();
}

template<class T> void freeSTL(T begin, T end)
{
	for (T i = begin; i != end; i++)
	{
		delete *i;
	}
}

template<class T, class S> void freeMap(std::map<T, S> &stl)
{
	for (typename std::map<T, S>::iterator i = stl.begin(); i != stl.end(); i++)
	{
		delete i->second;
	}

	stl.clear();
}

template<class T, class S> void freeHash(stdext::hash_map<T, S> &stl)
{
	for (typename stdext::hash_map<T, S>::iterator i = stl.begin(); i != stl.end(); i++)
	{
		delete i->second;
	}

	stl.clear();
}


// Owner pointer list helper classes
template<class T>
class HeapSTL : public T
{
public:
	void clear()
	{
		for (typename T::iterator i = this->begin(); i != this->end(); i++)
		{
			delete *i;
		}

		T::clear();
	}

	~HeapSTL()
	{
		for (typename T::iterator i = this->begin(); i != this->end(); i++)
		{
			delete *i;
		}
	}
};

template<class T, class S>
class HeapAssoc : public T
{
public:
	void clear()
	{
		for (typename stdext::hash_map<T, S>::iterator i = this->begin(); i != this->end(); i++)
		{
			delete i->second;
		}

		T::clear();
	}

	~HeapAssoc()
	{
		for (typename stdext::hash_map<T, S>::iterator i = this->begin(); i != this->end(); i++)
		{
			delete i->second;
		}
	}
};

// Helper functions
// split - divides a string into an array based on the given separators
inline void split(const std::string& src, std::vector<std::string>& dest, const std::string& separators)
{
	std::string::const_iterator cutBegin = src.begin();
	std::string::const_iterator cutEnd = src.begin();
	while (cutEnd < src.end())
	{
		if (std::find(separators.begin(), separators.end(), *cutEnd) != separators.end())
		{
			if (cutBegin != cutEnd)
			{
				dest.resize(dest.size() + 1);
				dest.back().insert(dest.back().end(), cutBegin, cutEnd);
			}

			cutBegin = cutEnd + 1;
			cutEnd = cutBegin;
		}
		else
		{
			cutEnd++;
		}
	}

	// put remaining characters in
	if (cutBegin < src.end())
	{
		dest.resize(dest.size() + 1);
		dest.back().insert(dest.back().end(), cutBegin, src.end());
	}
}

/// Makes a string from a sequence, delimited with specified string
template <class it>
inline std::string join(const it& begin, const it& end, const std::string& delimiter)
{
	std::string r;

	if (begin == end)
		return r;

	for (it i = begin; i != end; ++i)
	{
		r += *i;
		if (i != end - 1)
			r += delimiter;
	}

	return r;
}

inline std::string join(const std::vector<std::string>& src, const std::string& delimiter)
{
	return join(src.begin(), src.end(), delimiter);
}

// refbind1st
// as with bind1st but allows binding to reference of a variable
template <class T>
class refbinder1st : public std::unary_function<typename T::second_argument_type, typename T::result_type>
{
public:
	refbinder1st(const T& func, typename T::first_argument_type& v) :
	  op(func), value(v)
	{
	}

	typename T::result_type operator()(const typename T::second_argument_type& in) const
	{
		return (op(value, in));
	}

	typename T::result_type operator()(typename T::second_argument_type& in) const
	{
		return (op(value, in));
	}

protected:
	T op;	// the functor to apply
	typename T::first_argument_type& value;	// the left operand
};

template <class T, class U>
inline refbinder1st<T> refbind1st(const T& f, U& v) { return refbinder1st<T>(f, v); }

// refbind2nd
// as with bind2nd but allows binding to reference of a variable
template <class T>
class refbinder2nd : public std::unary_function<typename T::first_argument_type, typename T::result_type>
{
public:
	refbinder2nd(const T& func, typename T::second_argument_type& v) :
	  op(func), value(v)
	{
	}

	typename T::result_type operator()(const typename T::first_argument_type& in) const
	{
		return (op(in, value));
	}

	typename T::result_type operator()(typename T::first_argument_type& in)
	{
		return (op(in, value));
	}

protected:
	T op;	// the functor to apply
	typename T::second_argument_type& value;	// the left operand
};

template <class T, class U>
inline refbinder2nd<T> refbind2nd(const T& f, U& v) { return refbinder2nd<T>(f, v); }

template <class T, class U>
inline refbinder2nd<T> refbind2nd(const T& f, const U& v) { return refbinder2nd<T>(f, const_cast<typename T::second_argument_type&>(v)); }

#endif
