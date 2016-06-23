#include "Complex.h"
#include "Reflector.h"
#include <stdexcept>
//#include <boost/swap.h>

Complex::Complex()
{
}

Complex::Complex(const string& name,DataWapper data)
	:mName(name),mData(data)
{
}

Complex::~Complex(void)
{
}

Complex::Complex(const Complex& other)
	:mName(other.mName),mData(other.mData),mChildren(other.mChildren)
{
}

bool Complex::isValid() const
{
	return !mData.empty();
}

string Complex::name() const
{
	return mName;
}

size_t Complex::type() const
{
	if(!isValid())
		return 0;
	return mData.type().hash_code();
}

string Complex::typeName() const
{
	if(!isValid())
		return "";
	return mData.type().name();
}

int Complex::childrenCount() const
{
	return mChildren.size();
}

Complex Complex::child( int index ) const
{
	if(index>=childrenCount())
		boost::throw_exception(std::out_of_range("Access Complex's children out of range."));
	return mChildren.at(index);
}

Complex Complex::child( const string& name ) const
{
	Container::const_iterator it;
	for(it = mChildren.cbegin();it!=mChildren.cend();++it)
	{
		if((*it).mName == name)
		{
			return *it;
		}
	}
	return Complex();
}

void Complex::setName(const string& name)
{
	mName = name;
}

void Complex::addChild( Complex child)
{
	mChildren.push_back(child);
}

void Complex::removeChild( int index )
{
	Container::iterator it;
	int i = 0;
	for(it = mChildren.begin();it!=mChildren.end();++it,i++)
	{
		if(index == i)
		{
			mChildren.erase(it);
			break;
		}
	}
}

void Complex::removeChild( const string& name )
{
	Container::iterator it;
	for(it = mChildren.begin();it!=mChildren.end();++it)
	{
		if((*it).mName == name)
		{
			mChildren.erase(it++);
		}
	}
}

Complex& Complex::operator=(const Complex& other)
{
	mName = other.mName;
	mData = other.mData;
	mChildren = other.mChildren;
	return *this;
}

std::ostream& operator<<( std::ostream& os,const Complex& c )
{
	os << c.mName;
	os << c.typeName();
	c.mData.pack(os);
	Complex::Container::const_iterator it = c.mChildren.cbegin();
	for (; it != c.mChildren.end(); ++it)
	{
		os << *it;
	}
	return os;
}

std::istream& operator>>( std::istream& is,Complex& c )
{
	is >> c.mName;
	string anyType;
	is >> anyType;
	c.mData = Reflector::get_mutable_instance().create(anyType);
	c.mData.unpack(is);
	Complex::Container::iterator it = c.mChildren.begin();
	for (; it != c.mChildren.end(); ++it)
	{
		is >> *it;
	}
	return is;
}