// ---------------------------------------------------------------------------------------------------------
//
// Config
// Config variables are loaded lazily, on their use by other classes.
//
// Initially, all key/value pairs are read from the config file on disk and stored internally in a string->value
// map. At this point, the types of the config values are not known.
//
// The type of a variables is resolved when a user-class accesses the variable. At that point, the value
// is known as it is provided by the caller, and then the string representation of the variable is converted and
// stored as the native type by serialising from a stringstream into the new variable. From this point onwards,
// the native type is cached and can be returned directly without needing to re-serialise from the string
// representation.
//
// If a key/value pair is defined in the config file yet is not accessed during the course of the
// program's execution, then at exit time the untranslated string representation is simple written back to
// the options file. Resolved key/value pairs are converted back to strings, then written to the file.
//
// An explicit call must be made in order to update and save the value of an options variable.
//
// * Local Files
// Local versions of config files can be stored under the "localini" directory. Values in the
// "localini" version of a file override values in the master version found in the game's root folder.
// Only categories and values overriden in the "localini" file are written back to the "localini" file on save.
//
// ---------------------------------------------------------------------------------------------------------
#pragma once


#include "Standard/api.h"
#include "Dirty.h"
#include "ConfigService.h"
#include "RTTI.h"

class Config;

class STANDARD_API Config
{
public:
	Config();

	// Convenience constructor -- makes a config with a service reading from the given filesystem path and loads it.
	Config(const Path& path);

	void add(ConfigService& service);

	// construct config from hash of hash
	template <class T>
	Config(const std::string& outputFileName, const stdext::hash_map<std::string, stdext::hash_map<std::string, T> >& input);

	~Config();

	void load();

	std::string description() const;

	/// Returns true if a value in the config has been modified.
	bool dirty() const;

	// load current contents of config into hash of hash
	template <class T>
	void load(stdext::hash_map<std::string, stdext::hash_map<std::string, T> >& target);

	typedef std::vector<const ConfigService::ValueData*> Vars;

	struct CategoryVarPair
	{
		CategoryVarPair() {}

		CategoryVarPair(const ConfigService::CategoryData* _category) :
			category(_category)
		{}

		const ConfigService::CategoryData* category;
		Vars vars;

		bool operator < (const CategoryVarPair& rhs) const
		{
			return category->name < rhs.category->name;
		}

		bool operator == (const std::string& s) const
		{
			return category->name == s;
		}
	};

	typedef std::vector<class ConfigService*> ConfigServices;
	typedef std::vector<CategoryVarPair> Categories;

	const Categories& categories() const;

	// remove a config variable and prevent it from being saved
	void remove(const std::string& category, const std::string& name = "");

	// returns true if the variable exists so far
	bool exists(const std::string& category, const std::string& name = "") const;

	// Get a config variable using the rtti class name as the category. Searches the base class category if no variable or category
	// is found. If categoryOverride is specified, then that category is selected before the rtti category.
	template <class T>
	void get(const RTTI& rtti, const std::string& name, const std::string& categoryOverride, T& var) const
	{
		ensureConfigLoaded();

		if (!categoryOverride.empty())
		{
			// this could be faster
			if (exists(categoryOverride, name))
			{
				get<T>(categoryOverride, name, var);
				return;
			}
		}

		if (exists(rtti.className(), name) || rtti.isBase())
			get<T>(rtti.className(), name, var);
		else
			get<T>(*rtti.base(), name, categoryOverride, var);
	}

	template <class T>
	T get(const RTTI& rtti, const std::string& name, const std::string& categoryOverride, const T& defaultVal) const
	{
		ensureConfigLoaded();

		T newVar(defaultVal);
		get<T>(rtti, name, categoryOverride, newVar);
		return newVar;
	}

	// get a config variable - fill var with the value defined in the file if present,
	// else create a config value and set it to the value of var
	template <class T>
	void get(const std::string& category, const std::string& name, T& var) const
	{
		for (uint i = 0; i < m_services.size(); ++i)
		{
			T* p = m_services[i]->getVar<T>(category, name);
			if (p)
			{
				var = *p;
				return;
			}
		}

		m_services[0]->create(category, name, &var);
	};

	// get a config variable - return the value defined in the file if present,
	// else store and return the given default value
	template <class T>
	const T& get(const std::string& category, const std::string& name, const T& defaultVal) const
	{
		ensureConfigLoaded();

		for (uint i = 0; i < m_services.size(); ++i)
		{
			T* p = m_services[i]->getVar<T>(category, name);
			if (p)
				return *p;
		}

		// Note: is this const cast ok?
		return m_services[0]->create<T>(category, name, const_cast<T*>(&defaultVal));
	};

	// get a config variable - return the value defined in the file if present,
	// else store and return the result of the default constructor
	template <class T>
	const T& get(const std::string& category, const std::string& name) const
	{
		ensureConfigLoaded();

		for (uint i = 0; i < m_services.size(); ++i)
		{
			T* p = m_services[i]->getVar<T>(category, name);
			if (p)
				return *p;
		}

		return m_services[0]->create<T>(category, name);
	};

	// update a config variable
	template <class T>
	void set(const std::string& category, const std::string& name, const T& var)
	{
		onSet();

		// first try setting in a writable service that already contains the value
		for (uint i = 0; i < m_services.size(); ++i)
		{
			if (!m_services[i]->readOnly() && m_services[i]->setVar(category, name, var))
				return;
		}

		// no existing writable service found -- create the variable in the first available writable service
		if (!m_services.empty())
		{
			for (uint i = 0; i < m_services.size(); ++i)
			{
				if (!m_services[i]->readOnly())
				{
					_categoriesCache.setDirty();
					// note: is this const cast ok?
					m_services[i]->create<T>(category, name, const_cast<T*>(&var));
					return;
				}
			}
		}

		onSetError(category, name);
	};

	/// For each category this config has in common with 'src', performs the 'import' operation (if any) that was
	/// made for the category from 'src'.
	void linkImports(Config& src);

	// Direct access to config services, if required. Use the 'get', 'set' and 'category' methods in preference.
	ConfigServices& services();
	const ConfigServices& services() const;

private:
	void ensureConfigLoaded() const;
	void onSet();
	void onSetError(const std::string& category, const std::string& name) const;
	void buildNameCache() const;

	bool _dirty;
	bool _loaded;

	ConfigServices			m_services;

	mutable Dirty<Categories>	_categoriesCache;
};

// construct config from hash of hash
// note -- avoid saving configs created with this function. inherited categories will save all their parent's values too,
// which defeats the import semantics.
// tbd: not currently used, and code needs updating
/*template <class T>
Config::Config(const stdext::hash_map<std::string, stdext::hash_map<std::string, T> >& input)
{
	createServices();

	for (typename stdext::hash_map<std::string, stdext::hash_map<std::string, T> >::const_iterator i = input.begin(); i != input.end(); ++i)
	{
		for (typename stdext::hash_map<std::string, T>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			set(i->first, j->first, j->second);
		}
	}
}*/

// include "ConfigService.h" to use this function.
template <class T>
void Config::load(stdext::hash_map<std::string, stdext::hash_map<std::string, T> >& target)
{
	assert(!m_services.empty());
	for (typename ConfigServices::reverse_iterator i = m_services.rbegin(); i != m_services.rend(); ++i)
		(*i)->load(target);
}
