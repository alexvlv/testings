/*
$Id$

g++ unique_ptr.cpp -std=c++14 -o unique_ptr && ./unique_ptr

*/

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <stdlib.h>
#include <vector>

#define xstr(s)   str(s)
#define str(s)	  #s

#define DESRIPT(NAME) std::string desc = #NAME"~~~~~~";
#define PRNDESCPRIMO() \
{std::cerr << __PRETTY_FUNCTION__ << ':' << desc << ':' << static_cast<const void *>(this->desc.data()) << ':' << static_cast<const void *>(this->vector.data()) << std::endl;}

class Primo 
{
public:
	Primo(std::string desc = "Primo_default"):desc(desc),vector{1,2,3}{ PRNDESCPRIMO(); }
	Primo(const Primo &from):desc(from.desc),vector(from.vector){ PRNDESCPRIMO(); }
	Primo(Primo &&from):desc(std::move(from.desc)),vector(std::move(from.vector)){ PRNDESCPRIMO(); }
	Primo& operator =(const Primo &from) { 
		desc = from.desc + "_copy"; vector = from.vector;
		PRNDESCPRIMO(); 
		return *this;
	}
	~Primo(){ PRNDESCPRIMO();}
private:
	DESRIPT(Primo)
	std::vector<int> vector;
	friend std::ostream& operator<<(std::ostream& os, const Primo& pr);
};

std::ostream& operator<<(std::ostream& os, const Primo& pr)
{
	os <<  __PRETTY_FUNCTION__ << ':' << pr.desc << ':' << static_cast<const void *>(pr.desc.data()) << ':' << static_cast<const void *>(pr.vector.data());
	return os;
}

static Primo create_class()
{
	Primo prm{__PRETTY_FUNCTION__};
	return prm;
}

int main()
{
	std::cerr << "Smart pointers test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	if(true) {
		//const Primo& pr = create_class();
		Primo&& pr = create_class();
		std::cerr << pr << std::endl;
	}
	
	if(true) {	
		std::cerr << "====" << std::endl;
		std::vector<std::unique_ptr<Primo>> vec {};
		
		auto p = std::make_unique<Primo>();
		//vec.push_back(std::move(p));
		vec.push_back(static_cast<std::unique_ptr<Primo> &&>(p));
	}	
	return 0;
}
