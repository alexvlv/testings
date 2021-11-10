/*
$Id$

g++ non_virtual_interface.cpp -std=c++11 -o non_virtual_interface && ./non_virtual_interface

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
    void interface();

private:
	virtual void f() = 0;
	DESRIPT(Base)
};

// Never called
void Base::f() { std::cerr << "Pure virtual:" << __PRETTY_FUNCTION__ << std::endl; }

void Base::interface() { 
    std::cerr << __PRETTY_FUNCTION__ << " start \n****" << std::endl;
    f();
    std::cerr << "****\n" <<__PRETTY_FUNCTION__ << " end " << std::endl;
}

class Derived : public Base
{
public:
	CONSTRUCTOR(Derived)
	DESTRUCTOR(Derived)

private:
	virtual void f() { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
	
    DESRIPT(Derived)
};

int main()
{
	std::cerr << "Class using test" << std::endl
			  << "$Id$" << std::endl
			  << "Compiled: " __DATE__ " " __TIME__ << std::endl;

	Derived d;
	d.interface();
}

/*
Мэйерс С. - Эффективное использование C++. 55 верных советов улучшить структуру и код ваших программ (2006).pdf
Совет 35
non-virtual interface idiom - NVI

*/