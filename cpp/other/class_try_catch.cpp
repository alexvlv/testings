/*
$Id$

g++ class_try_catch.cpp -std=c++17 -o class_try_catch && ./class_try_catch

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

protected:

private:
	DESRIPT(Base)
};

class Derived : public Base
{
public:
	//CONSTRUCTOR(Derived)
    Derived(std::string desc ="Derived exception") : desc(desc) { PRNDESC();  throw std::runtime_error("error"); }
	DESTRUCTOR(Derived)

private:
	DESRIPT(Derived)
};

int main()
{
	std::cerr << "Derived class try/catch test" << std::endl
			  << "$Id$" << std::endl
			  << "Compiled: " __DATE__ " " __TIME__ << std::endl;
try {
	Derived d;
} catch(const std::exception& e) {
     std::cerr  << __PRETTY_FUNCTION__ << " catch: [" << e.what() << "]"; 
}

}
