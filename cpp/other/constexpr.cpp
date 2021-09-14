/*
$Id$

g++ constexpr.cpp -std=c++11 -o constexpr && ./constexpr

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

template <typename T>
constexpr T func(const T &a, const T &b) {return a+b;}

int main()
{
	std::cerr << "constexpr test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	constexpr auto a = func(1,2);
	decltype(a) b = a;
	std::cerr << b << std::endl;
	return 0;
}
