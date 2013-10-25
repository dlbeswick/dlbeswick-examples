#pragma once

template <class T>
class DereferenceLessThan
{
public:
	bool operator () (const T& lhs, const T& rhs)
	{
		return *lhs < *rhs;
	}
};

#include <vector>

// binary sorted vector
template<class T, class Predicate = std::less<T> >
class SortedVec : public std::vector<T>
{
public:
	SortedVec(const Predicate& p = Predicate()) :
	  m_predicate(p)
	{
	}


	typedef std::vector<T> BaseType;

	typename BaseType::iterator insert(const T& x)
	{
		return BaseType::insert(std::upper_bound(this->begin(), this->end(), x, m_predicate), x);
	}

	typename BaseType::iterator remove(const T& x)
	{
		typename BaseType::iterator i = this->find(x);
		if (i != this->end())
			return this->erase(i);

		return i;
	}

	typename BaseType::iterator find(const T& x)
	{
		typename BaseType::iterator begIt = this->begin();
		typename BaseType::iterator endIt = this->end();
		typename BaseType::iterator i = std::lower_bound(begIt, endIt, x, m_predicate);
		if (i != this->end() && !(x < *i))
			return i;
		else
			return this->end();
	}

	uint lower_bound(const T& x)
	{
		return std::lower_bound(this->begin(), this->end(), x) - this->begin();
	}

	uint upper_bound(const T& x)
	{
		return std::upper_bound(this->begin(), this->end(), x) - this->begin();
	}

private:
	Predicate m_predicate;
};
