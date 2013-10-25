// ------------------------------------------------------------------------------------------------
//
// ConfigService
// See Config.h
// allows transparent use of multiple ini files
//
// ------------------------------------------------------------------------------------------------
#pragma once

#include "Standard/api.h"
#include "streamops.h"
#include "Path.h"
#include "SmartPtr.h"

class STANDARD_API ConfigService
{
public:
	ConfigService(const Path& fileName, bool readOnly);
	~ConfigService();

	void load();
	void save(otextstream& stream);

	const std::string& description() const { return *m_fileName; }

	/// Used when saving config files, to determine ordering of categories between services.
	void setOrder(int order);

	struct sDataBase
	{
		virtual ~sDataBase() {}
		virtual sDataBase* clone() const = 0;

		bool bTypeResolved;

		virtual void write(class otextstream& s) const
		{
		}
	};

	class ConfigData
	{
	public:
		uint order;
		std::string name; // tbd: use custom sort operator to removing duplicate storage of keys in category/var maps?

		virtual ~ConfigData() {}

		bool operator < (const ConfigData& rhs) const
		{
			return name < rhs.name;
		}

	protected:
		friend class SortByOrder;

		ConfigData(uint _order, const ConfigService* _service, const std::string& _name) :
			order(_order), service(_service), name(_name)
		{
			// if you reached this point, then maybe you constructed an instance derived from this
			// class through a hash map access.
			assert(service);
		}

		const ConfigService* service;
	};

	class CategoryData;

	class ValueData : public ConfigData
	{
	public:
		friend class CategoryData;
		friend class stdext::hash_map<std::string, ValueData>;

		virtual void write(otextstream& s) const
		{
			this->value->write(s);
		}

		template <class T>
		T& get() const
		{
			if (!this->value->bTypeResolved)
			{
				sData<T>* newData = new sData<T>;

				SmartPtr<sData<std::string> > strData = value.downcast<sData<std::string> >();

				itextstream tss(*strData->var);
				tss << itextstream::whitespace("");
				tss << itextstream::str_delimiter("");
				tss >> *newData->var;

				newData->bTypeResolved = true;
				value = newData;
			}

			SmartPtr<sData<T> > resolvedData = value.downcast<sData<T> >();
			return *resolvedData->var;
		}

	protected:
		// This constructor is only for hash maps' compilation.
		ValueData() :
			 ConfigData(UINT_MAX, 0, std::string()) { assert(0); }

		ValueData(const ConfigService* s, uint order, sDataBase* _value, const std::string& name) :
			ConfigData(order, s, name), value(_value) {}

		ValueData(const ConfigService* s, const ValueData& data) :
			ConfigData(data.order, s, data.name), value(data.value)
		{
		}

		mutable SmartPtr<sDataBase> value;
	};

	typedef stdext::hash_map<std::string, ValueData> ConfigVals;

	class CategoryData : public ConfigData
	{
	public:
		ConfigVals values;
		CategoryData* parent;

		template <class T>
		T& create(const std::string& name, T* var = 0)
		{
			sData<T>* newData;
			if (var)
				newData = new sData<T>(*var);
			else
				newData = new sData<T>;

			newData->bTypeResolved = true;

			values.insert(ConfigVals::value_type(name, ValueData(service, UINT_MAX, newData, name)));

			return *newData->var;
		}

		template <class T>
		T* getVar(const std::string& name)
		{
			ConfigVals::iterator it = values.find(name);
			if (it == values.end())
			{
				// variable not found, try parent
				if (parent)
					return parent->getVar<T>(name);
				else
					return 0;
			}

			return &it->second.get<T>();
		}

		void createUnresolved(const std::string& name, const std::string& newVal, uint valueOrder);

		// set entry
		template <class T>
		bool setVar(const std::string& name, const T& newVal)
		{
			ConfigVals::iterator it = values.find(name);

			// return false if no such entry
			if (it == values.end())
			{
				return false;
			}

			ValueData& valueData = it->second;
			const SmartPtr<sDataBase>& value = valueData.value;

			if (!value->bTypeResolved)
			{
				resolveValueType<T>(valueData, new sData<T>(newVal));
			}
			else
			{
				// insert new object
				SmartPtr<sData<T> > resolvedData = value.downcast<sData<T> >();
				*resolvedData->var = newVal;
			}

			return true;
		}

		template <class T>
		void resolveValueType(ValueData& valueData, sDataBase* newData)
		{
			SmartPtr<sDataBase>& value = valueData.value;

			assert(!value->bTypeResolved);
			newData->bTypeResolved = true;

			// insert new object
			value = newData;
		}

	protected:
		friend class ConfigService;
		friend class stdext::hash_map<std::string, CategoryData>;

		CategoryData(const ConfigService* s = 0, uint _order = UINT_MAX, const std::string& name = std::string()) :
			ConfigData(_order, s, name), parent(0)
		{
		}
	};

	typedef stdext::hash_map<std::string, CategoryData> Categories;

	const Categories& categories() const { return m_vars; }

	// remove a config variable and prevent it from being saved
	void remove(const std::string& category, const std::string& name = "");

	// returns true if the variable exists so far
	bool exists(const std::string& category, const std::string& name = "");

	// create category
	Categories::iterator create(const std::string& category)
	{
		return m_vars.insert(Categories::value_type(category, CategoryData(this, UINT_MAX, category))).first;
	}

	// create entry
	template <class T>
	T& create(const std::string& category, const std::string& name, T* var = 0)
	{
		CategoryData& cat = create(category)->second;
		return cat.create(name, var);
	}

	// get entry
	template <class T>
	T* getVar(const std::string& category, const std::string& name)
	{
		// get the cached value from options file
		Categories::iterator it = m_vars.find(category);
		if (it == m_vars.end())
			return 0;

		CategoryData& catData = it->second;

		// try getting variable
		T* result = catData.getVar<T>(name);
		if (result)
			return result;

		// no variable present
		return 0;
	}

	// set entry
	template <class T>
	bool setVar(const std::string& category, const std::string& name, const T& newVal)
	{
		Categories::iterator it = m_vars.find(category);

		if (it == m_vars.end())
			it = m_vars.insert(Categories::value_type(category, CategoryData(this, UINT_MAX, category))).first;

		CategoryData& catData = it->second;
		return catData.setVar(name, newVal);
	}

	// load into a hash
	template <class T>
	void load(stdext::hash_map<std::string, stdext::hash_map<std::string, T> >& target)
	{
		for (Categories::const_iterator i = m_vars.begin(); i != m_vars.end(); ++i)
		{
			const CategoryData* parent = &i->second;
			while (parent)
			{
				for (ConfigVals::const_iterator j = parent->values.begin(); j != parent->values.end(); ++j)
				{
					target[i->first][j->first] = j->second.get<T>();
				}

				parent = parent->parent;
			}
		}
	}

	// see Config.h
	void linkImports(ConfigService& src);

	bool readOnly() const;
	uint order() const { return m_order; }

protected:
	template <class T>
	struct sData : public sDataBase
	{
		sData() :
			var(new T)
		{}

		sData(const T& rhs) :
			var(new T(rhs))
		{}

		~sData()
		{
			delete var;
		}

		T* var;

		virtual sDataBase* clone() const
		{
			return new sData<T>(*var);
		}

		virtual void write(otextstream& s) const
		{
			s << *var;
		}
	};

	typedef std::vector<CategoryData*> CategoryList;
	void getInheritedCategories(CategoryList& c);

	Categories		m_vars;
	Path			m_fileName;
	uint			m_order;
	bool			_readOnly;
};
