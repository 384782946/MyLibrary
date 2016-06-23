#pragma once

#include <boost/lexical_cast.hpp>
#include <iostream>

class Serializer
{
public:
	Serializer(/*std::iostream& stream*/);
	~Serializer(void);

	template<typename T>
	static T fromString(const char* src)
	{
		return boost::lexical_cast<T>(src);
	}

	template<typename T>
	static T fromString(const wchar_t* src)
	{
		return boost::lexical_cast<T>(src);
	}

	template<typename T>
	void operator<<(const T& obj)
	{
		//mBuff<<obj;
	}

	template<typename T>
	void operator>>(T& obj)
	{
		//mBuff>>obj;
	}

private:
	//std::iostream mBuffer;
};