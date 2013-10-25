// ---------------------------------------------------------------------------------------------------------
//
// Streamable
// Base class for streamable objects
//
// Set the "m_version" variable to define the version of your class.
//
// (optional) By overriding the "streamVars" function, you can build a list of the variables to be automatically
// streamed.
//
// Use the STREAMVAR macro within that function to acheive this.
//
// Streaming by this method is backwards compatible, in that removed and added variables will be ignored.
// For more complex versioning functionality, overload the read/write functions.
//
// Binary streams must save the name of each variable in order to be backwards compatible, so don't use the
// automatic method if space is an issue.
//
// ---------------------------------------------------------------------------------------------------------
#ifndef STANDARD_STREAMABLE_H
#define STANDARD_STREAMABLE_H

#include "Standard/api.h"
#include "binstream.h"
#include "textstream.h"
#include "streamops.h"
#include "streamops_math.h"
#include "Exception.h"
#include "SmartPtr.h"
#include "RTTI.h"

class Config;

// StreamVarBase
class STANDARD_API StreamVarBase
{
public:
	StreamVarBase(const std::string& name);

	virtual ~StreamVarBase();

	const std::string& name() const;

	// text writing
	std::string toString() const;

	void fromString(const std::string& inputStr);

	virtual void setDefault() = 0;

	virtual void write(obinstream& s) const = 0;
	virtual void write(otextstream& s) const = 0;
	virtual void read(ibinstream& s) = 0;
	virtual void read(itextstream& s) = 0;

protected:
	std::string m_name;
};

template <class T>
class StreamVar : public StreamVarBase
{
public:
	StreamVar(const std::string& name, T& var, const T& defaultValue) :
	  StreamVarBase(name),
	  _data(&var),
	  _default(new T(defaultValue))
	{
	}

	StreamVar(const std::string& name, T& var) :
	  StreamVarBase(name),
	  _data(&var),
	  _default(0)
	{
	}

	~StreamVar()
	{
		delete _default;
	}

	T& var() { return *_data; }
	const T& var() const { return *_data; }

	virtual void setDefault()
	{
		if (_default)
			var() = *_default;
	}

	virtual void write(obinstream& s) const
	{
		s << var();
	}

	virtual void write(otextstream& s) const
	{
		s << otextstream::enclose_str(true);
		s << var();
		s << otextstream::enclose_str();
	}

	virtual void read(ibinstream& s)
	{
		s >> var();
	}

	virtual void read(itextstream& s)
	{
		s >> var();
	}

protected:
	T* _default;
	T* _data;
};

// streamvar void implementation
/*void*& StreamVar<void*>::var() { return (void*&)0; }
const void*& StreamVar<void*>::var() const { return (void*&)0; }*/

template<> class StreamVar<void*>
{
	void write(obinstream& s) const {}
	void write(otextstream& s) const {}
	void read(ibinstream& s) {}
	void read(itextstream& s) {}
};

template<> inline void StreamVar<std::string>::read(itextstream& s)
{
	s.skipWhitespace();
	if (s.has("\""))
		s << itextstream::enclose_str(true);
	else
	{
		// hack: stop unquoted strings swallowing input
		// this should at least occur on call instead of here
		s << itextstream::str_delimiter(",\n\r)");
	}

	s >> var();

	s << itextstream::enclose_str();
}

// use this to add vars within the streamVars function
template <class T> inline StreamVarBase* makeStreamVar(const std::string& name, T& var) { return new StreamVar<T>(name, var); }
template <class T> inline StreamVarBase* makeStreamVar(const std::string& name, T& var, const T& defaultVar) { return new StreamVar<T>(name, var, defaultVar); }

#define STREAMVAR(x)			v.push_back(makeStreamVar(#x, x))
#define STREAMVARDEFAULT(x,d)	v.push_back(makeStreamVar(#x, x, d))
#define STREAMVARNAME(name, x)	v.push_back(makeStreamVar(#name, x))

// Streamable
class STANDARD_API Streamable
{
public:
	Streamable();

	virtual ~Streamable() {}
	typedef std::vector<SmartPtr<StreamVarBase> > StreamVars;

	int version() const { return m_version; }

	// binary writing
	virtual void write(obinstream& s, const StreamVars& v) const;
	virtual void read(ibinstream& s, const StreamVars& v);
	virtual void write(obinstream& s) const;
	virtual void read(ibinstream& s);

	// text writing
	virtual void write(otextstream& s, const StreamVars& v) const;
	virtual void read(itextstream& s, const StreamVars& v);
	virtual void write(otextstream& s) const;
	virtual void read(itextstream& s);

	// writing to and from configs
	virtual void write(Config& c, const std::string& category, const StreamVars& v) const;
	virtual void read(const Config& c, const std::string& category, const StreamVars& v);
	virtual void write(Config& c, const std::string& category) const;
	virtual void read(const Config& c, const std::string& category);
	virtual void read(const Config& c, const RTTI& rtti, const std::string& categoryOverride);

	// text writing
	std::string toString() const;

protected:
	// notification
	virtual void onPreRead() {};
	virtual void onPostRead() {};

	virtual void read(const Config& c, const std::string& category, const StreamVars& v, const RTTI* rtti);
	virtual void readVarFromConfig(StreamVarBase& var, const Config& c, const std::string& category, const RTTI* rtti);

	// Called during saving or loading to retrieve streamVars.
	virtual void getStreamVars(StreamVars& v);

	// StreamVars should be defined in derived classes here.
	virtual void streamVars(StreamVars& v);

	int m_version;

private:
	// compile hash map of available vars
	void availableVarsHash(const StreamVars& v, stdext::hash_map<std::string, StreamVarBase*>& map);
};

// stream operators
inline otextstream& operator << (otextstream& s, Streamable& rhs) { rhs.write(s); return s; }
inline itextstream& operator >> (itextstream& s, Streamable& rhs) { rhs.read(s); return s; }
inline obinstream& operator << (obinstream& s, Streamable& rhs) { rhs.write(s); return s; }
inline ibinstream& operator >> (ibinstream& s, Streamable& rhs) { rhs.read(s); return s; }

// helpers
#define USE_STREAMING \
	protected: virtual void streamVars(StreamVars& v);
// Write this before the stream call:
//#define METADATA <classname>
#define STREAM \
	void METADATA::streamVars(StreamVars& v) \
	{ \
		Super::streamVars(v);
// STREAMVAR(varname)
// ...
#define END_STREAM \
	}

#endif
