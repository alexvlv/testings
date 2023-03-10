/*
$Id$

g++ virtual.cpp -std=c++11 -o virtual && ./virtual

*/

#include <iostream>

using namespace std;

struct T1 {
	virtual int f1() const {
		return 1;
	}

	int f2() const {
		return 1;
	}
};

struct T2 : T1 {
	int f1() const {
		return 2;
	}

	int f2() const {
		return 2;
	}
};

void f(T1 * p1) {
	cout << p1->f1() << "  " << p1->f2() << endl;
}

int main(){
	T2 t2;
	f( &t2 );
	return 0;
}

