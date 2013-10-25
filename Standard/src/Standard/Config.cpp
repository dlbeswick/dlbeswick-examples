// ---------------------------------------------------------------------------------------------------------
// 
// Config
//
// Implementation Details
// "Config" objects contain a list of "ConfigService" objects. Each "ConfigService" object contains a set of
// categories containing keys and values. Values from ConfigService objects earlier in the list "shadow"
// values from later in the list. The Config object is simply a wrapper around the list of ConfigService objects.
//
// Missing options
// In the event that an option is found in any of the ConfigServices, the option will be created in the head
// ConfigService.
// 
// ---------------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "Config.h"
#include "ConfigService.h"
#include "STLHelp.h"
#include "Path.h"
#include "Exception/Config.h"
#include <fstream>

// construct
Config::Config() :
	_loaded(false),
	_dirty(false)
{
}

Config::Config(const Path& path) :
	_loaded(false),
	_dirty(false)
{
	add(*new ConfigService(path, false));
	load();
}

Config::~Config()
{
	freeSTL(m_services);
}

// query category and variable names
class SortByOrder
{
public:
	inline bool operator() (const Config::CategoryVarPair& lhs, const Config::CategoryVarPair& rhs) const
	{
		return operator()(lhs.category, rhs.category);
	}

	inline bool operator() (const ConfigService::ConfigData* lhs, const ConfigService::ConfigData* rhs) const
	{
		return lhs->order < rhs->order && lhs->service->order() <= rhs->service->order();
	}
};

class SortByName
{
public:
	inline bool operator() (const ConfigService::ConfigData* lhs, const ConfigService::ConfigData* rhs) const
	{
		return *lhs < *rhs;
	}
};

#define TempVars std::set<const ConfigService::ValueData*, SortByName>
#define TempCats std::map<const ConfigService::CategoryData*, TempVars, SortByName>

void Config::buildNameCache() const
{
	_categoriesCache->clear();
	
	TempCats temp;

	// Compile all categories and variables into maps and sets.
	// When resolving variables from different services, the variable from the latest service is used.
	// This code also traverses category linkages resulting from 'import from...' statements.
	for (uint i = 0; i < m_services.size(); ++i)
	{
		const ConfigService::Categories& c = m_services[i]->categories();
		for (ConfigService::Categories::const_iterator i = c.begin(); i != c.end(); ++i)
		{
			const ConfigService::CategoryData* catData = &i->second;
			TempVars& f = temp[catData];

			while (catData)
			{
				const ConfigService::ConfigVals& v = catData->values;
				for (ConfigService::ConfigVals::const_iterator j = v.begin(); j != v.end(); ++j)
				{
					std::pair<TempVars::iterator, bool> result = f.insert(&j->second);
					if (!result.second)
					{
						f.erase(result.first);
						f.insert(&j->second);
					}
				}

				catData = catData->parent;
			}
		}
	}

	// fill vectors
	for (TempCats::iterator i = temp.begin(); i != temp.end(); ++i)
	{
		Vars& vars = _categoriesCache->insert(_categoriesCache->end(), CategoryVarPair(i->first))->vars;
		for (TempVars::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			vars.push_back(*j);
		}
	}

	// sort categories and variables by integer order. this is important if code depends on the order that
	// variables are defined in the ini.
	std::sort(_categoriesCache->begin(), _categoriesCache->end(), SortByOrder());
	for (Categories::iterator i = _categoriesCache->begin(); i != _categoriesCache->end(); ++i)
	{
		Vars& v = i->vars;
		
		std::sort(v.begin(), v.end(), SortByOrder());
	}

	_categoriesCache.setDirty(false);
}

void Config::load()
{
	if (m_services.empty())
		EXCEPTIONSTREAM(ExceptionConfigSave(*this), "Config save was requested, but no services have been added to the config.");

	_loaded = true;
	_dirty = false;

	std::for_each(m_services.begin(), m_services.end(), std::mem_fun<void, ConfigService>(&ConfigService::load));

	_categoriesCache.setDirty();
}

void Config::remove(const std::string& category, const std::string& name)
{
	for (uint i = 0; i < m_services.size(); ++i)
		m_services[i]->remove(category, name);

	_categoriesCache.setDirty();
}

bool Config::exists(const std::string& category, const std::string& name) const
{
	for (uint i = 0; i < m_services.size(); ++i)
		if (m_services[i]->exists(category, name))
			return true;

	return false;
}

const Config::Categories& Config::categories() const
{
	if (_categoriesCache.dirty())
		buildNameCache();

	return *_categoriesCache;
}

void Config::add(ConfigService& service)
{
	service.setOrder(m_services.size());
	m_services.push_back(&service);
}

void Config::linkImports(Config& src)
{
	for (uint i = 0; i < m_services.size(); ++i)
	{
		m_services[i]->linkImports(*src.m_services[i]);
	}
}

std::string Config::description() const
{
	if (m_services.empty())
		return "Empty.";

	return m_services[0]->description();
}

void Config::onSet()
{
	_dirty = true;
}

bool Config::dirty() const
{
	return _dirty;
}

void Config::onSetError(const std::string& category, const std::string& name) const
{
	EXCEPTIONSTREAM(ExceptionConfig(*this), "All services are read-only, so the value could not be set. (" << category << "/" << name << ")");
}

void Config::ensureConfigLoaded() const
{
	if (!_loaded)
		EXCEPTIONSTREAM(ExceptionConfig(*this), "The config was used without being loaded.");
}

Config::ConfigServices& Config::services()
{
	return m_services;
}

const Config::ConfigServices& Config::services() const
{
	return m_services;
}
