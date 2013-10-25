// ------------------------------------------------------------------------------------------------
//
// text/binstream global operators
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "Standard/binstream.h"
#include "Standard/textstream.h"
#include "Standard/Exception.h"
#include "Standard/platform.h"
#include <vector>
#include <map>

// global operators

// STL
// std::string
inline otextstream& operator << (otextstream& s, const std::string& obj)
{
	if (s.m_enclose_str.m_bEnclose)
	{
		s.s() << "\"";

		std::string newStr = obj;
		// replace quotes with escaped quotes
		for (unsigned int i = 0; i < newStr.size(); i++)
		{
			if (newStr[i] == '"')
				if ((i > 0 && newStr[i-1] != '\\') || i == 0)
					newStr.insert(newStr.begin() + i, '\\');
		}

		s.s().write(newStr.c_str(), (unsigned int)newStr.size());

		s.s() << "\"";
	}
	else
	{
		s.s().write(obj.c_str(), (unsigned int)obj.size());
	}

	return s;
}

// std::vector
template <class T>
inline otextstream& operator << (otextstream& s, const std::vector<T>& obj)
{
	s << otextstream::enclose_str(true);

	s.s() << "[";

	for (unsigned int i = 0; i < obj.size(); i++)
	{
		if (i)
			s.s() << ", ";

		s << obj[i];
	}

	s.s() << "]";

	s << otextstream::enclose_str();

	return s;
}

template <class T>
inline itextstream& operator >> (itextstream& s, std::vector<T>& obj)
{
	obj.clear();

	if (!s.has("["))
		return s;

	s >> "[";

	s << itextstream::enclose_str(true);
	s.skipWhitespace("\r\n\t");

	if (!s.has("]"))
	{
		do
		{
			obj.resize(obj.size() + 1);

			s.skipWhitespace("\r\n\t ");
			s >> obj.back();

			s.skipWhitespace("\r\n\t ");
			if (s.has(","))
				s >> ",";
			else
				break;
		}
		while (true);
	}

	s.skipWhitespace("\r\n\t ");
	s >> "]";

	s << itextstream::enclose_str();

	return s;
}

// hash_map
template <class S, class T>
inline otextstream& operator << (otextstream& s, const stdext::hash_map<S, T>& obj)
{
	s << otextstream::enclose_str(true);

	s.s() << "[";

	for (typename stdext::hash_map<S, T>::const_iterator i = obj.begin(); i != obj.end(); ++i)
	{
		if (i != obj.begin())
			s.s() << ", ";

		s << i->first;
		s.s() << "=";
		s << i->second;
	}

	s.s() << "]";

	s << otextstream::enclose_str();

	return s;
}

template <class S, class T>
inline itextstream& operator >> (itextstream& s, stdext::hash_map<S, T>& obj)
{
	s >> "[";

	s << itextstream::str_delimiter("\n");
	s << itextstream::whitespace();
	s.skipWhitespace();

	if (!s.has("]"))
	{
		do
		{
			S key;
			T val;

			s << itextstream::enclose_str(false);
			s << itextstream::str_delimiter("=");
			s >> key;
			s >> s.m_delimiter;

			s << itextstream::enclose_str(true);
			s >> val;

			obj[key] = val;

			s.skipWhitespace("\t\n ");

			if (s.has(","))
				s >> ",";
			else
				break;
		}
		while (true);
	}

	s.skipWhitespace("\t\r\n ");
	s >> "]";

	s << itextstream::enclose_str();

	return s;
}

// support for basic types
inline itextstream& operator >> (itextstream& s, float& f)
{
	s.s() >> f;
	return s;
}

inline itextstream& operator >> (itextstream& s, int& i)
{
	s.s() >> i;
	return s;
}

inline itextstream& operator >> (itextstream& s, unsigned int& i)
{
	s.s() >> i;
	return s;
}

inline itextstream& operator >> (itextstream& s, int64& i)
{
	s.s() >> i;
	return s;
}

inline itextstream& operator >> (itextstream& s, bool& b)
{
	if (s.has("true"))
	{
		b = true;
		s.s().seekg(4, std::ios::cur);
		return s;
	}
	else if (s.has("false"))
	{
		b = false;
		s.s().seekg(5, std::ios::cur);
		return s;
	}

	b = false;
	return s;
}

inline itextstream& operator >> (itextstream& s, void*& c)
{
	s.s() >> c;
	return s;
}

/*inline otextstream& operator << (otextstream& s, const size_t& e)
{
        s.s() << e;
        return s;
}*/

inline otextstream& operator << (otextstream& s, const float& f)
{
	s.s() << f;
	return s;
}

inline otextstream& operator << (otextstream& s, const bool& b)
{
	if (b)
		s.s() << "true";
	else
		s.s() << "false";

	return s;
}

inline otextstream& operator << (otextstream& s, const int& i)
{
	s.s() << i;
	return s;
}

inline otextstream& operator << (otextstream& s, const unsigned int& i)
{
	s.s() << i;
	return s;
}

#if !IS_WINDOWS
inline otextstream& operator << (otextstream& s, size_t i)
{
        s.s() << i;
        return s;
}
#endif

inline otextstream& operator << (otextstream& s, const int64& i)
{
	s.s() << i;
	return s;
}

inline otextstream& operator << (otextstream& s, const char* c)
{
	if (c && c[0] == '\n')
		s << s.endl;

	s.s() << c;
	return s;
}

inline otextstream& operator << (otextstream& s, const void*& c) // reference because on read, auto-conversion to ptr ref not allowed
{
	s.s() << c;
	return s;
}

// BINARY //////////////////////////////////////////////////////////////////////////////////////////

// bool
inline obinstream& operator << (obinstream& s, const bool& b)
{
	s.s().write((char*)&b, sizeof(bool));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, bool& b)
{
	s.s().read((char*)&b, sizeof(bool));
	return s;
}

// char
inline obinstream& operator << (obinstream& s, const char& c)
{
	s.s().write(&c, 1);
	return s;
}

inline ibinstream& operator >> (ibinstream& s, char& c)
{
	s.s().read(&c, 1);
	return s;
}

// uchar
inline obinstream& operator << (obinstream& s, const unsigned char& c)
{
	s.s().write((char*)&c, 1);
	return s;
}

inline ibinstream& operator >> (ibinstream& s, unsigned char& c)
{
	s.s().read((char*)&c, 1);
	return s;
}

// float
inline obinstream& operator << (obinstream& s, const float& f)
{
	s.s().write((char*)&f, sizeof(float));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, float& f)
{
	s.s().read((char*)&f, sizeof(float));
	return s;
}

// short
inline obinstream& operator << (obinstream& s, const short& i)
{
	s.s().write((char*)&i, sizeof(short));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, short& i)
{
	s.s().read((char*)&i, sizeof(short));
	return s;
}

inline obinstream& operator << (obinstream& s, const unsigned short& i)
{
	s.s().write((char*)&i, sizeof(unsigned short));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, unsigned short& i)
{
	s.s().read((char*)&i, sizeof(unsigned short));
	return s;
}

// unsigned long
inline obinstream& operator << (obinstream& s, const unsigned long& i)
{
	s.s().write((char*)&i, sizeof(unsigned long));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, unsigned long& i)
{
	s.s().read((char*)&i, sizeof(unsigned long));
	return s;
}

// int
inline obinstream& operator << (obinstream& s, const int& i)
{
	s.s().write((char*)&i, sizeof(int));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, int& i)
{
	s.s().read((char*)&i, sizeof(int));
	return s;
}

// int64
inline obinstream& operator << (obinstream& s, const int64& i)
{
	s.s().write((char*)&i, sizeof(int64));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, int64& i)
{
	s.s().read((char*)&i, sizeof(int64));
	return s;
}

// unsigned int64
#if IS_OSX // hack, fix
inline obinstream& operator << (obinstream& s, const uint64& i)
{
	s.s().write((char*)&i, sizeof(uint64));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, uint64& i)
{
	s.s().read((char*)&i, sizeof(uint64));
	return s;
}
#endif

// unsigned int
inline obinstream& operator << (obinstream& s, const unsigned int& i)
{
	s.s().write((char*)&i, sizeof(unsigned int));
	return s;
}

inline ibinstream& operator >> (ibinstream& s, unsigned int& i)
{
	s.s().read((char*)&i, sizeof(unsigned int));
	return s;
}

// pointer
inline obinstream& operator << (obinstream& s, const void*& c)  // reference because on read, auto-conversion to ptr ref not allowed
{
	s.s() << c;
	return s;
}

inline ibinstream& operator >> (ibinstream& s, void*& c)
{
	s.s() >> c;
	return s;
}

// STL
// string
inline obinstream& operator << (obinstream& s, const std::string& obj)
{
	s.s().write(obj.c_str(), (unsigned int)obj.size() + 1);

	return s;
}

inline ibinstream& operator >> (ibinstream& s, std::string& obj)
{
	int c;

	obj.clear();

	do
	{
		c = s.s().get();
		if (c > 0)
		{
			obj.resize(obj.size() + 1);
			obj[obj.size() - 1] = c;
		}
		else
			break;
	} while (s.s());

	return s;
}

// vector
template <class T>
inline obinstream& operator << (obinstream& s, const std::vector<T>& obj)
{
	s << obj.size();
	for (unsigned int i = 0; i < obj.size(); ++i)
	{
		s << obj[i];
	}

	return s;
}

template <class T>
inline ibinstream& operator >> (ibinstream& s, std::vector<T>& obj)
{
	unsigned int size;
	s >> size;
	if (!s.s())
		throwf("Bad read of vector");

	obj.resize(size);

	for (unsigned int i = 0; i < obj.size(); ++i)
	{
		s >> obj[i];
	}

	return s;
}

// map
template <class S, class T>
inline obinstream& operator << (obinstream& s, const std::map<S, T>& obj)
{
	s << obj.size();
	for (typename std::map<S, T>::const_iterator i = obj.begin(); i != obj.end(); ++i)
	{
		s << i->first << i->second;
	}

	return s;
}

template <class S, class T>
inline ibinstream& operator >> (ibinstream& s, std::map<S, T>& obj)
{
	unsigned int size;
	S varS;
	T varT;

	obj.clear();

	s >> size;
	if (!s.s())
		throwf("Bad read of vector");
	for (unsigned int i = 0; i < size; i++)
	{
		s >> varS >> varT;
		obj.insert(std::pair<S, T>(varS, varT));
	}

	return s;
}

// hash_map
template <class S, class T>
inline obinstream& operator << (obinstream& s, const stdext::hash_map<S, T>& obj)
{
	s << obj.size();
	for (typename stdext::hash_map<S, T>::const_iterator i = obj.begin(); i != obj.end(); ++i)
	{
		s << i->first << i->second;
	}

	return s;
}

template <class S, class T>
inline ibinstream& operator >> (ibinstream& s, stdext::hash_map<S, T>& obj)
{
	unsigned int size;
	S varS;
	T varT;

	obj.clear();

	s >> size;
	if (!s.s())
		throwf("Bad read of vector");
	for (unsigned int i = 0; i < size; i++)
	{
		s >> varS >> varT;
		obj.insert(std::pair<S, T>(varS, varT));
	}

	return s;
}

