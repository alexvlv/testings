/*
$Id$

g++ class_ops.cpp -std=c++11 -o class_ops && ./class_ops

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
#define CONSTRUCTORCOPY(NAME) \
NAME(const NAME &from):desc(from.desc){ PRNDESC(); }
#define CONSTRUCTORMOVE(NAME) \
NAME(const NAME &&from):desc(std::move(from.desc)){ PRNDESC(); }

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
	CONSTRUCTORCOPY(Primo)
	CONSTRUCTORMOVE(Primo)
	DESTRUCTOR(Primo)
private:
	DESRIPT(Primo)
};

class Secundo 
{
public:
	CONSTRUCTOR(Secundo)
	Secundo(const Secundo &from):desc(from.desc),mprimo(from.mprimo){ PRNDESC(); }
	Secundo(const Secundo &&from):desc(std::move(from.desc)),mprimo(std::move(from.mprimo)){ PRNDESC(); }
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
	if(false) {	
		std::cerr << "====" << std::endl;
		Secundo secundo;
		Secundo seccopy = secundo;
		Secundo moved = std::move(secundo);
	}	
	if(true) {	
		std::cerr << "====" << std::endl;
		Secundo secundo {xstr(secundo)};
		//Secundo moved = std::move(secundo);
		Secundo moved (std::move(secundo));
		std::cerr << "++++" << std::endl;
	}	
	return 0;
}

/*
Demangling:
std::cerr << typeid(Primo).name() << std::endl;
https://stackoverflow.com/questions/3649278/how-can-i-get-the-class-name-from-a-c-object
*/
