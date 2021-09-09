/*
$Id$

g++ class_rvalue.cpp -std=c++11 -o class_rvalue && ./class_rvalue

*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <limits>
#include <stdlib.h>
#include <vector>
#include <cassert>

#define xstr(s)   str(s)
#define str(s)	  #s

#define PRNDESC() \
{std::cerr << __PRETTY_FUNCTION__ << ':' << desc << ':' << static_cast<const void *>(this->desc.data()) << std::endl;}

#define CONSTRUCTOR(NAME) \
NAME(std::string desc = #NAME"_default"):desc(desc){ PRNDESC(); }
#define DESTRUCTOR(NAME) ~NAME(){ PRNDESC();}
#define DESRIPT(NAME) std::string desc = #NAME"~~~~~~";

#define CLASS(NAME) \
class NAME \
{ \
public: \
	CONSTRUCTOR(NAME) \
	DESTRUCTOR(NAME) \
private: \
	DESRIPT(NAME) \
}; \

CLASS(Macro)

class Primo 
{
public:
	CONSTRUCTOR(Primo)
	DESTRUCTOR(Primo)
private:
	DESRIPT(Primo)
};

class Secundo 
{
public:
	CONSTRUCTOR(Secundo)
	DESTRUCTOR(Secundo)
private:
	DESRIPT(Secundo)
	Primo mprimo {"member"};
};
 
int main()
{
	std::cerr << "Class create/copy/move test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	if(false) {
		Macro macro;
		std::cerr << typeid(Macro).name() << std::endl;
	}	
	
	if(false) {	
		std::cerr << "====" << std::endl;
		Primo primo;
		Primo primocopy = primo;
	}	
	if(true) {	
		std::cerr << "====" << std::endl;
		Secundo secundo;
		Secundo seccopy = secundo;
	}	
	return 0;
}

/*
Demangling:
std::cerr << typeid(Primo).name() << std::endl;
https://stackoverflow.com/questions/3649278/how-can-i-get-the-class-name-from-a-c-object
*/
