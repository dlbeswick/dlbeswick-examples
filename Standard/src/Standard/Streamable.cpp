// ------------------------------------------------------------------------------------------------
//
// Streamable
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Streamable.h"
#include "Config.h"
#include "Help.h"
#include "Log.h"

#define VALIDATE _DEBUG

////

StreamVarBase::StreamVarBase(const std::string& name) : 
	m_name(name)
{
}

StreamVarBase::~StreamVarBase() 
{
}

const std::string& StreamVarBase::name() const 
{ 
	return m_name; 
}

std::string StreamVarBase::toString() const
{
	std::stringstream* strstream = new std::stringstream;

	otextstream stream((std::ostream*)strstream);
	write(stream);

	return strstream->str();
}

void StreamVarBase::fromString(const std::string& inputStr)
{
	if (inputStr.empty())
	{
		setDefault();
	}
	else
	{
		itextstream stream(inputStr);
		read(stream);
	}
}

////

Streamable::Streamable() : 
	m_version(1)
{
}

void Streamable::streamVars(StreamVars& v)
{
	STREAMVAR(m_version);
}

// binary writing
void Streamable::write(obinstream& s) const
{
	StreamVars v;
	const_cast<Streamable*>(this)->getStreamVars(v);
	write(s, v);
}

void Streamable::read(ibinstream& s)
{
	StreamVars v;
	getStreamVars(v);
	read(s, v);
}

void Streamable::write(obinstream& s, const StreamVars& v) const
{
	s << v.size();
	for (uint i = 0; i < v.size(); ++i)
	{
		s << v[i]->name();
		v[i]->write(s);
	}
}

void Streamable::read(ibinstream& s, const StreamVars& v)
{
	uint size;
	s >> size;

	// compile hash map of available vars
	stdext::hash_map<std::string, StreamVarBase*> map;
	availableVarsHash(v, map);

	for (uint i = 0; i < size; ++i)
	{
		std::string name;
		s >> name;

		// check a variable of that name exists and load it in
		stdext::hash_map<std::string, StreamVarBase*>::iterator found = map.find(name);
		if (found != map.end())
		{
			found->second->read(s);
		}
	}

	onPostRead();
}

// text writing
void Streamable::write(otextstream& s) const
{
	StreamVars v;
	const_cast<Streamable*>(this)->getStreamVars(v);
	write(s, v);
}

void Streamable::read(itextstream& s)
{
	StreamVars v;
	getStreamVars(v);
	read(s, v);
}

void Streamable::write(otextstream& s, const StreamVars& v) const
{
	s.tabIn();

	s << "(";

	s.nextLine();

	for (uint i = 0; i < v.size(); ++i)
	{
		s << v[i]->name() << "=";
		v[i]->write(s);
	
		if (i < v.size() - 1)
		{
			s << ",";
			s.nextLine();
		}
	}

	s.nextLine();
	s.tabOut();

	s << ")";
}

void Streamable::read(itextstream& s, const StreamVars& v)
{
	s >> "(";

	// compile hash map of available vars
	stdext::hash_map<std::string, StreamVarBase*> map;
	availableVarsHash(v, map);

	do
	{ 
		std::string name;
		s << itextstream::str_delimiter("=");
		s << itextstream::whitespace("\r\n\t");
		s >> name;
		s >> s.m_delimiter;

		if (name.empty())
			break;

		// check that variable of that name exists as a STREAMVAR definition and load it in
		stdext::hash_map<std::string, StreamVarBase*>::iterator found = map.find(name);
		if (found != map.end())
		{
			found->second->read(s);
		}
		else
		{
			// throw: to catch in debugger
			try
			{
				throwf("Streamable variable defined in file not found in class, skipping: " + name + "\n");
			}
			catch (Exception& e)
			{
				derr << e.what();
			}

			// var not found, read until next var def
			while (!s.has(",") && !s.has(")"))
				s.s().seekg(1, std::ios::cur);
		}

		if (s.has(","))
			s >> ",";
		if (s.has(")"))
		{
			break;
		}
	}
	while (s);

//	s.skipWhitespace("\t\r\n ");
	s >> ")";

	onPostRead();
}

// writing to and from configs
void Streamable::write(Config& c, const std::string& category) const
{
	StreamVars v;
	const_cast<Streamable*>(this)->getStreamVars(v);
	write(c, category, v);
}

void Streamable::read(const Config& c, const RTTI& rtti, const std::string& categoryOverride)
{
	StreamVars v;
	getStreamVars(v);
	read(c, categoryOverride, v, &rtti);
}

void Streamable::read(const Config& c, const std::string& category)
{
	StreamVars v;
	getStreamVars(v);
	read(c, category, v, 0);
}

void Streamable::write(Config& c, const std::string& category, const StreamVars& v) const
{
	if (category.empty())
	{
		dwarn << "Streamable::write - Can't write config when empty category name given" << dlog.endl;
		return;
	}

	for (uint i = 0; i < v.size(); ++i)
	{
		c.set(category, v[i]->name(), v[i]->toString());
	}
}

void Streamable::readVarFromConfig(StreamVarBase& var, const Config& c, const std::string& category, const RTTI* rtti)
{
	std::string data;
	if (rtti)
		c.get<std::string>(*rtti, var.name(), category, data);
	else
		c.get<std::string>(category, var.name(), data);

	if (!data.empty())
	{
		itextstream s(data);
		
		s << itextstream::str_delimiter("\n");
		s << itextstream::whitespace("");

		var.read(s);
	}
	else
	{
		var.setDefault();
	}
}

void Streamable::read(const Config& c, const std::string& category, const StreamVars& v)
{
	read(c, category, v, 0);
}

void Streamable::read(const Config& c, const std::string& category, const StreamVars& v, const RTTI* rtti)
{
	if (!rtti && category.empty())
	{
		dwarn << "Streamable::read - Can't read config when empty category name given" << dlog.endl;
		return;
	}

	// compile hash map of available vars
	stdext::hash_map<std::string, StreamVarBase*> map;
	availableVarsHash(v, map);

	if (!rtti)
	{
		if (!c.exists(category))
		{
			dwarn << "Streamable::read - No category named " << category << " found, object may not yet exist in config file" << dlog.endl;
			return;
		}
	}

	for (StreamVars::const_iterator i = v.begin(); i != v.end(); ++i)
	{ 
		readVarFromConfig(**i, c, category, rtti);
		// fix: if the variable is missing from the config, no matter
	}

	onPostRead();
}

// compile hash map of available vars
void Streamable::availableVarsHash(const StreamVars& v, stdext::hash_map<std::string, StreamVarBase*>& map)
{
	for (uint i = 0; i < v.size(); ++i)
	{
		map[v[i]->name()] = v[i].ptr();
	}
}

void Streamable::getStreamVars(StreamVars& v)
{
	streamVars(v);
	
	// check for duplicates
#if VALIDATE
	/*StreamVars newVars = v;
	newVars.erase(newVars.begin(), std::unique(newVars.begin(), newVars.end()));
	assert(newVars.size() == v.size());*/
#endif
}

std::string Streamable::toString() const
{
	std::stringstream* strstream = new std::stringstream;

	otextstream stream((std::ostream*)strstream);
	write(stream);

	return strstream->str();
}
