/*
$Id$

g++ string_move.cpp -std=c++11 -o string_move && ./string_move

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

#define PRINTSTR(S) { std::cerr << '['  << S << "]:" << S.size() << ':'  << static_cast<const void *>(S.data()) << std::endl; }

 
int main()
{
	std::cerr << "String move test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	if(true) {	
		std::cerr << "====" << std::endl;
		std::string src{"Source 77dbd99a9e548f47bd185f72da0c8ee9b88d0122 77dbd99a9e548f47bd185f72da0c8ee9b88d0122"};
		std::cerr <<  "src: " << std::endl;
		PRINTSTR(src);
		std::string moved (std::move(src));
		std::cerr <<  "moved: " << std::endl;
		PRINTSTR(moved);
		std::cerr <<  "src after move: " << std::endl;
		PRINTSTR(src);
		std::cerr << "++++" << std::endl;
	}	
	return 0;
}

/*
Demangling:
std::cerr << typeid(Primo).name() << std::endl;
https://stackoverflow.com/questions/3649278/how-can-i-get-the-class-name-from-a-c-object
*/
