/*
$Id$

g++ class_pure_virtual.cpp -std=c++11 -o class_pure_virtual && ./class_pure_virtual

*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <limits>
#include <cstdlib>
#include <vector>
#include <cassert>

#define xstr(s) str(s)
#define str(s) #s

#define PRNDESC()                                                                                                            \
	{                                                                                                                        \
		std::cerr << __PRETTY_FUNCTION__ << ':' << desc << ':' << static_cast<const void *>(this->desc.data()) << std::endl; \
	}

#define CONSTRUCTOR(NAME) \
	NAME(std::string desc = #NAME "_default") : desc(desc) { PRNDESC(); }
#define DESTRUCTOR(NAME) \
	~NAME() { PRNDESC(); }
#define CONSTRUCTORCOPY(NAME) \
	NAME(const NAME &from) : desc(from.desc) { PRNDESC(); }
#define CONSTRUCTORMOVE(NAME) \
	NAME(const NAME &&from) : desc(std::move(from.desc)) { PRNDESC(); }

#define DESRIPT(NAME) std::string desc = #NAME "~~~~~~";

class Base
{
public:
	CONSTRUCTOR(Base)
	DESTRUCTOR(Base)
	virtual void f() = 0;

protected:
	virtual void g() = 0;

private:
	DESRIPT(Base)
};

void Base::f() { std::cerr << "Pure virtual:" << __PRETTY_FUNCTION__ << ':' << std::endl; }
void Base::g() { std::cerr << "Pure virtual:" << __PRETTY_FUNCTION__ << ':' << std::endl; }

class Derived : public Base
{
public:
	CONSTRUCTOR(Derived)
	DESTRUCTOR(Derived)

	virtual void f() { std::cerr << __PRETTY_FUNCTION__ << ':' << std::endl; }
	virtual void g()
	{
		std::cerr << __PRETTY_FUNCTION__ << "CALLING:" << std::endl;
		Base::f();
	}

private:
	DESRIPT(Derived)
};

int main()
{
	std::cerr << "Class using test" << std::endl
			  << "$Id$" << std::endl
			  << "Compiled: " __DATE__ " " __TIME__ << std::endl;

	Derived d;
	d.Base::f();
	d.g();
}

/*
Мэйерс С. - Эффективное использование C++. 55 верных советов улучшить структуру и код ваших программ (2006).pdf
Совет 34

C++ Supports pure virtual functions with an implementation so class designers can force derived classes to override 
the function to add specific details , but still provide a useful default implementation that they can use as a common base. 
https://stackoverflow.com/questions/39388152/why-does-c-support-for-a-pure-virtual-function-with-an-implementation/39388910#:~:text=C%2B%2B%20Supports%20pure%20virtual%20functions,use%20as%20a%20common%20base.&text=If%20you%20define%20it%20as,class%20must%20implement%20the%20function.

*/