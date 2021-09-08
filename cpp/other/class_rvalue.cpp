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


class SomeClass 
{
public:
	SomeClass(std::string desc = "default"):desc(desc){ std::cerr << "SomeClass(): " << desc << std::endl;}
	~SomeClass(){ std::cerr << "~SomeClass(): " << desc << std::endl;}
private:
	std::string desc = "~~~~~~";
};

int main()
{
	std::cerr << "Class create/copy/move test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	

	SomeClass smc;

	return 0;
}
