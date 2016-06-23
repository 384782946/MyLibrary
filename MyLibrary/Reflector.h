#pragma once

#include <iostream>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/serialization/singleton.hpp>
//#include <boost/any.hpp>
#include "serializable_any.h"
#include <map>

using std::map;
using std::string;

#ifdef BOOST_NO_CXX11_LAMBDAS

#else
#define REGIST_REFLECT_TYPE(T,Name) Reflector::get_mutable_instance().regist(Name,[]{return DataWapper(T());});
#endif

typedef serializable_any DataWapper;
typedef boost::function<DataWapper()> ObjectFactory;

class Reflector:public boost::serialization::singleton<Reflector>
{
	typedef map<string, ObjectFactory> ObjectFactoryContainer;

public:
	Reflector(void)
	{
	}

	~Reflector(void)
	{
	}

	DataWapper create(const string& uniuqeName)
	{
		ObjectFactoryContainer::iterator it = mRegistedFactory.find(uniuqeName);
		if (it != mRegistedFactory.end())
		{
			return it->second();
		}
		else
		{
			return DataWapper();
		}
	}

	bool regist(const string& name,ObjectFactory factory)
	{
		ObjectFactoryContainer::iterator it = mRegistedFactory.find(name);
		if (it != mRegistedFactory.end())
		{
//#ifdef NDEBUG
//			return false;
//#else
			boost::throw_exception(std::logic_error(name + "has been registed."));
//#endif
		}

		mRegistedFactory[name] = factory;
		return true;
	}

private:
	ObjectFactoryContainer mRegistedFactory;
};