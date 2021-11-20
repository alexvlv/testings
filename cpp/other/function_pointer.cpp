/*
$Id$

g++ function_pointer.cpp -std=c++11 -o function_pointer && ./function_pointer

*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <limits>
#include <cstdlib>
#include <vector>
#include <functional>
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

class Functor 
{
public:
	CONSTRUCTOR(Functor)
    CONSTRUCTORCOPY(Functor) 
    CONSTRUCTORMOVE(Functor) 
	DESTRUCTOR(Functor) 
    
    void operator()() { std::cerr << __PRETTY_FUNCTION__ << ':' << std::endl; }
    virtual void operator()(int v) { std::cerr << __PRETTY_FUNCTION__ << ':' << v << std::endl; }

private:
	DESRIPT(Functor)
};

void call_Functor( Functor &f ) 
{ 
    std::cerr << __PRETTY_FUNCTION__ << "begin" << std::endl;
    f();
    std::cerr << __PRETTY_FUNCTION__ << "end" << std::endl;
}

void function() { std::cerr << __PRETTY_FUNCTION__  << std::endl; }

void call_function( void (*fp)()  )
{
    std::cerr << __PRETTY_FUNCTION__ << "begin" << std::endl;
    fp();
    std::cerr << __PRETTY_FUNCTION__ << "end" << std::endl;
}

void call_template( std::function<void()> &&ft )
{
    std::cerr << __PRETTY_FUNCTION__ << "begin" << std::endl;
    ft();
    std::cerr << __PRETTY_FUNCTION__ << "end" << std::endl;
}

template<class A, class B>
constexpr A func_tmpl(A first, B last)
{
    return first+last;
}

int main()
{
	std::cerr << "Function pointer using test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;

    Functor f;
    std::cerr << "====" << std::endl; 
    f(); 
    std::cerr << "====" << std::endl; 
    f(0);
    std::cerr << "++++" << std::endl; 
    call_Functor(f);
    std::cerr << "----" << std::endl;
    call_function(function);
    std::cerr << "%%%" << std::endl;
    call_template(f);
    call_template(function);
    std::cerr << "%%%" << std::endl;
    auto v = func_tmpl(1,5.0);
    std::cerr << "[" << v << "]" << std::endl;
}
