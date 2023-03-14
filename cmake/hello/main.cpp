/*
 $Id$

 g++ main.cpp -std=c++11 -o main && ./main
 
*/

#include "macro.h"
//#include ".git.h"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	cout << "Hello, world!" << endl;
	cout << xstr(SIGN(5)) << endl;
	exit(0);
}
