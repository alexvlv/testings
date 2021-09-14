/*
$Id$

g++ class_operator.cpp -std=c++11 -o class_operator && ./class_operator

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

class Primo ;
Primo operator+(const Primo &a, const Primo &b);

class Primo 
{
public:
	explicit Primo(int v = 0, char t = '*'):v(v),t(t){ std::cerr << __PRETTY_FUNCTION__ << ':' << this->t << ':' << this->v << std::endl; }
	~Primo(){ std::cerr << __PRETTY_FUNCTION__ << ':' << this->t << ':' << v << std::endl; }
 
	Primo operator+(const Primo &other) 
	{ 
		std::cerr << __PRETTY_FUNCTION__ << ':' << this->t << ':' << v << '+' << other.v << std::endl; 
		Primo res(v + other.v,'C'); 
		return res;
	}  
private:
	int v = -1;
	char t = ' ';
	friend Primo operator+(const Primo &a, const Primo &b);
};

Primo operator+(const Primo &a, const Primo &b)
{
		Primo res(a.v + b.v,'F'); 
		return res;
} 

int main()
{
	std::cerr << "Class operator test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	if(true) {	
		std::cerr << "====" << std::endl;
		Primo primo(33, 'A');
		primo = primo + Primo(1,'B');
	}	
	return 0;
}

/*
https://en.cppreference.com/w/cpp/language/operators
Operator Overloading on StackOverflow C++ FAQ
https://stackoverflow.com/questions/4421706/what-are-the-basic-rules-and-idioms-for-operator-overloading
*/
