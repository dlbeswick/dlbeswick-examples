// ------------------------------------------------------------------------------------------------
//
// Base
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "PtrGC.h"
#include "Base.h"
#include "Config.h"
#include "textstream.h"
#include "Exception/Base.h"

IMPLEMENT_RTTI(Base);

Base::Base() :
	m_currentConfig(0)
#if _DEBUG
	,_constructed(false)
#endif
{
}

Base::~Base()
{
}

const std::string& Base::configCategory() const
{
	if (m_configCategoryOverride.empty())
		return rtti().className();
	else
		return m_configCategoryOverride;
}

void Base::streamFromConfig()
{
	if (configCategory() != rtti().className())
		Streamable::read(config(), rtti(), configCategory());
	else
		Streamable::read(config(), rtti(), "");
}

void Base::writeRTTI(otextstream& s) const
{
	StreamVars v;
	const_cast<Base*>(this)->streamVars(v);

	s << otextstream::enclose_str(false);

	s.tabIn();
	s.nextLine();
	s << "(";

	s << rtti().className();

	if (!v.empty())
	{
		s << ",";
		Streamable::write(s);
	}

	s.nextLine();
	s.tabOut();

	s << ")";
}
Base* Base::readRTTI(itextstream& s)
{
	std::string rttitype;

	if (!s.has("("))
		return 0;

	s << itextstream::enclose_str(false);
	s << itextstream::str_delimiter(",)");
	s >> "(";

	s.skipWhitespace("\r\n\t");

	s << itextstream::str_delimiter(",\r\n(");
	s >> rttitype;

	if (s.has(","))
		s >> ",";

	s.skipWhitespace(" \r\n\t");

	Base* b = newObject(rttitype.c_str());
	if (!b)
		throwf("Base::readRTTI - Unknown rtti type " + rttitype + ", has REGISTER_RTTI been called?");
	b->Streamable::read(s);

	s.skipWhitespace(" \r\n\t");
	s >> ")";

	return b;
}

void Base::writeRTTI(obinstream& s) const
{
	s << rtti().className();
	Streamable::write(s);
}
Base* Base::readRTTI(ibinstream& s)
{
	std::string rttitype;
	s >> rttitype;

	Base* b = newObject(rttitype.c_str());
	if (!b)
		throwf("Base::readRTTI - Unknown rtti type " + rttitype);
	b->Streamable::read(s);
	return b;
}

void Base::update(float delta)
{
	counters.update(delta);
}

void Base::construct()
{
	assert(!_constructed);

	if (m_currentConfig)
	{
		streamFromConfig();
	}

	configLoad();

#if _DEBUG
	_constructed = true;
#endif
}

Base* Base::newObject(const RTTI& rtti, const std::string& configOverride)
{
	const Registrant* r = registrant(rtti);
	Base* newObj = (Base*)r->func->construct();

	if (newObj)
	{
		newObj->setConfigCategory(configOverride);
		if (!configOverride.empty() && !newObj->config().exists(configOverride))
			throwf("No config category found (for '" + newObj->config().description() + "') called " + configOverride);

		if (newObj->needPostNewObjectConstructCall())
			newObj->construct();
	}

	return newObj;
}

Base* Base::newObject(const std::string& className, const std::string& configOverride)
{
	const Registrant* r = registrant(className);
	if (!r)
		throw(ExceptionBaseNoRegistrant(className));

	Base* newObj = (Base*)r->func->construct();

	if (newObj)
	{
		if (!configOverride.empty())
			newObj->setConfigCategory(configOverride);

		if (!configOverride.empty() && !newObj->config().exists(configOverride))
			throwf("No config category found (for '" + newObj->config().description() + "') called " + configOverride);

		if (newObj->needPostNewObjectConstructCall())
			newObj->construct();
	}

	return newObj;
}

Base* Base::streamFromConfig(Config& config, const std::string& category)
{
	if (!config.exists(category))
	{
		dwarn << "Base::streamFromConfig - No category named " << category << " found, object may not yet exist in config file" << dlog.endl;
		return 0;
	}

	// read class type
	std::string sClass = config.get<std::string>(category, "class", std::string());
	if (sClass.empty())
	{
		derr << "Base::streamFromConfig -- no class defined when loading category '" + category + "'\n";
		return 0;
	}

	// create obj
	Base* newObj = newObject(sClass);
	if (!newObj)
	{
		derr << "Base::streamFromConfig -- couldn't construct object of type '" + sClass + "' when loading category '" + category + "'\n";
		return 0;
	}

	// load config into object
	newObj->setConfig(config);
	newObj->setConfigCategory(category);
	newObj->streamFromConfig();

	return newObj;
}

void Base::streamVars(StreamVars& v)
{
	Streamable::streamVars(v);
}

void Base::clearConfig()
{
	m_currentConfig = 0;
}

void Base::setConfig(Config& config, const std::string& category)
{
	m_currentConfig = &config;
	if (!category.empty())
		setConfigCategory(category);
}

Config& Base::config()
{
	if (!m_currentConfig)
		throwf("No config defined.");

	return *m_currentConfig;
}

const Config& Base::config() const
{
	if (!m_currentConfig)
		throwf("No config defined.");

	return *m_currentConfig;
}

void Base::setConfigCategory(const std::string& category)
{
	m_configCategoryOverride = category;
}

bool Base::hasConfig() const
{
	return m_currentConfig != 0;
}

bool Base::needPostNewObjectConstructCall() const
{
	return true;
}
