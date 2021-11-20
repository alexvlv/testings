/*
$Id$

g++ function_template.cpp -std=c++11 -o function_template && ./function_template

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



template<class A, class B>
A func_tmpl(A first, B last)
{
    return first+last;
}

int main()
{
	std::cerr << "Function pointer using test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;

    double v = func_tmpl(1,5.0);
    std::cerr << "[" << v << "]" << std::endl;
}
