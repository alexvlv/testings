/*
$Id$

g++ class_using.cpp -std=c++11 -o class_using && ./class_using

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

#define xstr(s)   str(s)
#define str(s)	  #s

#define PRNDESC() \
{std::cerr << __PRETTY_FUNCTION__ << ':' << desc << ':' << static_cast<const void *>(this->desc.data()) << std::endl;}

#define CONSTRUCTOR(NAME) \
NAME(std::string desc = #NAME"_default"):desc(desc){ PRNDESC(); }
#define DESTRUCTOR(NAME) ~NAME(){ PRNDESC();}
#define CONSTRUCTORCOPY(NAME) \
NAME(const NAME &from):desc(from.desc){ PRNDESC(); }
#define CONSTRUCTORMOVE(NAME) \
NAME(const NAME &&from):desc(std::move(from.desc)){ PRNDESC(); }

#define DESRIPT(NAME) std::string desc = #NAME"~~~~~~";

class Base 
{
public:
	CONSTRUCTOR(Base) 
	DESTRUCTOR(Base) 
    
    virtual void f() { std::cerr << __PRETTY_FUNCTION__ << ':' << std::endl; }
    virtual void f(int) { std::cerr << __PRETTY_FUNCTION__ << ':' << std::endl; }

private:
	DESRIPT(Base)
};

class Derived: public Base 
{
public:
	CONSTRUCTOR(Derived) 
	DESTRUCTOR(Derived) 
    using Base::f;
    void f() { std::cerr << __PRETTY_FUNCTION__ << ':' << std::endl; }

private:
	DESRIPT(Derived)
};


int main()
{
	std::cerr << "Class using test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;

    Derived d;
    d.f();
    d.f(0);

}

/*
Мэйерс С. - Эффективное использование C++. 55 верных советов улучшить структуру и код ваших программ (2006).pdf
Совет 33

*/