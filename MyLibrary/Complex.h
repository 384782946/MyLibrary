#pragma once

#include <string>
#include <vector>
#include "serializable_any.h"
//#include <boost/any.hpp>

using std::string;
using std::vector;

typedef serializable_any DataWapper;

class Complex
{
	typedef vector<Complex> Container;

	friend std::ostream& operator<<(std::ostream& os, const Complex& c);
	friend std::istream& operator>>(std::istream& is, Complex& c);

public:
	Complex();
	explicit Complex(const string& name,DataWapper data = DataWapper());
	Complex(const Complex&);
	~Complex(void);
	
	bool isValid() const;
	string name() const;
	size_t type() const;
	string typeName() const;
	inline int childrenCount() const;
	Complex child(int index) const;
	Complex child(const string& name) const;
	void setName(const string& name);
	void addChild(Complex);
	void removeChild(int index);
	void removeChild(const string& name);
	Complex& operator=(const Complex&);

	template<typename T>
	T value()
	{
		return boost::any_cast<T>(mData);
	}

	template<typename ValueType>
	Complex & operator=(const ValueType & rhs)
	{
		mData = rhs;
		return *this;
	}

private:
	string mName;
	DataWapper mData;
	Container mChildren;
};

std::ostream& operator<<(std::ostream& os,const Complex& c);
std::istream& operator>>(std::istream& is,Complex& c);