// MyLibrary.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "util.h"
#include "Complex.h"
#include "Reflector.h"
#include "serializable_any.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

class A
{
public:
	virtual ~A(){}

	virtual void say_hello()
	{
		std::cout<<"class A:say_hello"<<std::endl;
	}
};

class B:public A
{
public:
	virtual void say_hello()
	{
		std::cout<<"class B:say_hello"<<std::endl;
	}
};

class C:public A
{
public:
	virtual void say_hello()
	{
		std::cout<<"class C:say_hello"<<std::endl;
	}
};

template<typename T>
class D :public A
{
public:
	virtual void say_hello()
	{
		std::cout <<typeid(*this).name()<<":say_hello" << std::endl;
	}
};

class Other
{

};

int _tmain(int argc, _TCHAR* argv[])
{
	/*A* a = new C();
	try
	{
	B* b = polymorphic_cast<B*>(a);
	b->say_hello();
	}
	catch (...)
	{

	}

	A& aa = C();
	try
	{

	B& bb = polymorphic_cast<B&>(aa);
	bb.say_hello();
	}
	catch (...)
	{

	}*/
/*
	Complex parent("parent",2);
	Complex c1("child1",1.1);
	Complex c2("child2",string("hello world"));
	parent.addChild(c1);
	parent.addChild(c2);

	std::cout<<parent.name()<<"	"<<parent.typeName()<<" "<<parent.value<int>()<<std::endl;
	std::cout<<parent.child("child1").name()<<"	"<<parent.child("child1").typeName()<<" "<<parent.child("child1").value<double>()<<std::endl;
	std::cout<<parent.child(1).name()<<" "<<parent.child(1).typeName()<<" "<<parent.child(1).value<string>()<<std::endl;

	*/
	
	//REGIST_REFLECT_TYPE(int, "int");
	REGIST_REFLECT_TYPE(double, "double");
	Reflector::get_mutable_instance().regist("int", []{return DataWapper(int(110)); });
	Reflector::get_mutable_instance().regist("string", []{return DataWapper(string("empty string")); });

	auto ret = Reflector::get_mutable_instance().create("int");
	std::cout << ret.type().name() << any_cast<int>(ret) << std::endl;
	ret = Reflector::get_mutable_instance().create("double");
	std::cout << ret.type().name() << std::endl;
	ret = Reflector::get_mutable_instance().create("string");
	//std::cout << ret.type().name() << std::endl;
	std::cout << any_cast<std::string>(ret) << std::endl;
	std::cout << ret;
	
	return 0;
}

