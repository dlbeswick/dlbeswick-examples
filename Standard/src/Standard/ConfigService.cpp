// ------------------------------------------------------------------------------------------------
//
// ConfigService
//
// ------------------------------------------------------------------------------------------------
#include "Standard/pch.h"
#include "ConfigService.h"
#include "Filesystem.h"
#include <fstream>
#include "Exception/Filesystem.h"

void ConfigService::CategoryData::createUnresolved(const std::string& name, const std::string& newVal, uint valueOrder)
{
	sData<std::string>* data = new sData<std::string>;
	data->bTypeResolved = false;
	*data->var = newVal;
	values.insert(ConfigVals::value_type(name, ValueData(service, valueOrder, data, name)));
	++valueOrder;
}

ConfigService::ConfigService(const Path& fileName, bool readOnly) :
	m_fileName(fileName),
	m_order(0),
	_readOnly(readOnly)
{
}

ConfigService::~ConfigService()
{
}

void ConfigService::setOrder(int order)
{
	m_order = 0;
}

void ConfigService::load()
{
	uint categoryOrder = 0;

	SmartPtr<std::iostream> is;

	try
	{
		// open file stream
		is = m_fileName.open();
	}
	catch (ExceptionFilesystem&)
	{
		return;
	}

	itextstream txtstream(is);

	m_vars.clear();

	// prime options mgr with values in options file
	while (txtstream)
	{
		std::string category, key, val, valAppend, importFrom;

		txtstream << itextstream::whitespace();
		while (!txtstream.has("[", false) && txtstream)
		{
			txtstream.nextLine();
		}

		if (!txtstream)
			break;

		// read category
		txtstream << itextstream::str_delimiter("]");
		txtstream << itextstream::whitespace("\r\n\t");
		txtstream >> category >> txtstream.endl;

		if (!(txtstream))
			break;

		// create new category in hash map
		CategoryData& curCat = m_vars.insert(Categories::value_type(category, CategoryData(this, categoryOrder, category))).first->second;
		++categoryOrder;

		// read keys and values

		// import statement
		if (txtstream.has("import"))
		{
			try
			{
				txtstream << itextstream::str_delimiter("\n\r");
				txtstream >> "import from " >> importFrom;
				txtstream >> txtstream.m_delimiter;
			}
			catch (const Exception& e)
			{
				throwf("Syntax of 'import' command is 'import from categoryName' (" + e + ")");
			}

			// note: with new pointer-based inheritance system this may no longer be an issue
			Categories::iterator i = m_vars.find(importFrom);
			if (i == m_vars.end())
				throwf("'import' statement requested import from category '" + importFrom + "', but a category with that name has not been defined yet.");

			curCat.parent = &i->second;
		}

		// include inherited values when calculating object count
		uint currentValueCount = 0;
		const CategoryData* valueCountCat = curCat.parent;
		while (valueCountCat)
		{
			currentValueCount += valueCountCat->values.size();
			valueCountCat = valueCountCat->parent;
		}

		uint valueOrder = currentValueCount;

		while (!txtstream.has("[") && txtstream)
		{
			if (!isalpha(txtstream.s().peek()))
			{
				txtstream.nextLine();
				continue;
			}

			// read key
			txtstream << itextstream::whitespace();
			txtstream << itextstream::enclose_str(false);
			txtstream << itextstream::str_delimiter("=");
			txtstream >> key;
			txtstream >> txtstream.m_delimiter;

			if (!txtstream)
				break;

			// read val
			txtstream << itextstream::str_delimiter("\r\n");
			txtstream << itextstream::whitespace("");
			txtstream.skipWhitespace(" \t");

			val.clear();
			do
			{
				if (!val.empty())
					val += "\n"; // preserve line breaks in config string entries

				valAppend.clear();

				char c;
				do
				{
					c = txtstream.s().get();

					if (txtstream)
					{
						valAppend.push_back(c);

						if (c == '\r' || c == '\n')
						{
							do
							{
								c = txtstream.s().peek();

								if (!txtstream || (c != '\r' && c != '\n'))
									break;

								valAppend.push_back(c);

								txtstream.s().get();
							}
							while (true);

							break;
						}
					}
					else
					{
						break;
					}
				}
				while (true);

				val += valAppend;

				if (!txtstream || (!txtstream.has("\t") && !txtstream.has(")") && !txtstream.has("]")))
					break;
			}
			while (true);

			txtstream << itextstream::whitespace("\r\n");
			txtstream.skipWhitespace();

			if (!txtstream && !txtstream.s().eof())
				break;

			// chop whitespace
			while (!val.empty() && (val[val.size() - 1] == '\r' || val[val.size() - 1] == '\n'))
			{
				val.erase(val.end() - 1);
			}

			// insert into db
			curCat.createUnresolved(key, val, valueOrder);
			++valueOrder;
		}
	}
}

// sorting for ordered save
static bool operator < (const std::pair<ConfigService::CategoryData*, std::string>& lhs, const std::pair<ConfigService::CategoryData*, std::string>& rhs)
{
	if (lhs.first->order == rhs.first->order)
		return lhs.second < rhs.second;
	return lhs.first->order < rhs.first->order;
}

static bool operator < (const std::pair<ConfigService::ValueData*, std::string>& lhs, const std::pair<ConfigService::ValueData*, std::string>& rhs)
{
	if (lhs.first->order == rhs.first->order)
		return lhs.second < rhs.second;
	return lhs.first->order < rhs.first->order;
}

void ConfigService::save(otextstream& stream)
{
	bool bFirst = true;

	// order the categories to ensure ordering in file is maintained
	std::map<std::pair<CategoryData*, std::string>, std::multiset<std::pair<ValueData*, std::string> > > orderedValues;

	for (Categories::iterator i = m_vars.begin(); i != m_vars.end(); ++i)
	{
		// don't write empty categories
		if (i->second.values.empty())
			continue;

		std::multiset<std::pair<ValueData*, std::string> >& values = orderedValues[std::pair<CategoryData*, std::string>(&i->second, i->first)];

		ConfigVals& vals = i->second.values;
		for (ConfigVals::iterator j = vals.begin(); j != vals.end(); ++j)
		{
			values.insert(std::pair<ValueData*, std::string>(&j->second, j->first));
		}
	}

	// save file from memory
	otextstream os(stream);

	// write categories in order
	for (std::map<std::pair<CategoryData*, std::string>, std::multiset<std::pair<ValueData*, std::string> > >::iterator i = orderedValues.begin(); i != orderedValues.end(); ++i)
	{
		CategoryData& catData = *i->first.first;

		// write space between categories
		if (!bFirst)
			os << os.endl;
		else
			bFirst = false;

		os << "[" << i->first.second << "]" << os.endl;
		if (catData.parent)
			os << "import from " << catData.parent->name << os.endl;

		// write category var keys and values
		std::multiset<std::pair<ValueData*, std::string> >& valueNames = i->second;
		for (std::multiset<std::pair<ValueData*, std::string> >::iterator j = valueNames.begin(); j != valueNames.end(); j++)
		{
			os << j->second << " = ";
			j->first->write(os);
			os << os.endl;
		}
	}
}

// remove a config variable and prevent it from being saved
void ConfigService::remove(const std::string& category, const std::string& name)
{
	if (m_vars.find(category) == m_vars.end())
		return;

	// delete a single entry
	if (!name.empty())
	{
		Categories::iterator itCategory = m_vars.find(category);

		if (itCategory != m_vars.end())
		{
			itCategory->second.values.erase(name);
		}
	}
	else
	{
		// delete an entire category
		m_vars.erase(category);
	}
}

// returns true if the variable exists so far
bool ConfigService::exists(const std::string& category, const std::string& name)
{
	Categories::iterator it = m_vars.find(category);
	if (it == m_vars.end())
		return false;

	if (name.empty())
		return true;

	ConfigVals& configVals = it->second.values;
	ConfigVals::iterator itv = configVals.find(name);
	return itv != configVals.end();
}

bool ConfigService::readOnly() const
{
	return _readOnly;
}

void ConfigService::getInheritedCategories(CategoryList& c)
{
	for (Categories::iterator i = m_vars.begin(); i != m_vars.end(); ++i)
	{
		CategoryData& catData = i->second;

		if (catData.parent)
			c.push_back(&catData);
	}
}

void ConfigService::linkImports(ConfigService& src)
{
	CategoryList inherited;
	src.getInheritedCategories(inherited);

	for (uint i = 0; i < inherited.size(); ++i)
	{
		CategoryData& inheritedCat = *inherited[i];
		// find matching parent category
		Categories::iterator parentCatIt = m_vars.find(inheritedCat.parent->name);
		if (parentCatIt == m_vars.end())
			continue;

		// find or create matching category
		Categories::iterator catIt = m_vars.find(inheritedCat.name);
		if (catIt == m_vars.end())
			catIt = create(inheritedCat.name);

		if (!catIt->second.parent)
			catIt->second.parent = &parentCatIt->second;
	}
}
