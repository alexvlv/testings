// g++ -std=c++17 -O2 -Wall -pedantic -pthread main.cpp && ./a.out

#include <iostream>
#include <utility>

int y(int &) { return 1; }
int y(int &&) { return 2; }

template <class T> int f(T &&x) { return y(x); }
template <class T> int g(T &&x) { return y(std::move(x)); }
template <class T> int h(T &&x) { return y(std::forward<T>(x)); }

int main() {
  int i = 10;
  std::cout << f(i) << f(20);
  std::cout << g(i) << g(20);
  std::cout << h(i) << h(20);
  return 0;
}

/*
g++ -std=c++20 -O2 -Wall -pedantic -pthread main.cpp && ./a.out

https://cppquiz.org/quiz/question/116

The T&& in the templated functions do not necessarily denote an rvalue reference, it depends on the type that is used to instantiate the template. If instantiated with an lvalue, it collapses to an lvalue reference, if instantiated with an rvalue, it collapses to an rvalue reference. See note [1].

Scott Meyers has written a very good article about this, where he introduces the concept of "universal references" (the official term is "forwarding reference") http://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers

In this example, all three functions are called once with an lvalue and once with an rvalue. In all cases, calling with an lvalue (i) collapses T&& x to T& x (an lvalue reference), and calling with an rvalue (20) collapses T&& x to T&& x (an rvalue reference). Inside the functions, x itself is always an lvalue, no matter if its type is an rvalue reference or an lvalue reference.

-For the first example, y(int&) is called for both cases. Output: 11.
-For the second example, move(x) obtains an rvalue reference, and y(int&&)is called for both cases. Output: 22.
-For the third example, forward<T>(x) obtains an lvalue reference when x is an lvalue reference, and an rvalue reference when x is an rvalue reference, resulting in first a call to y(int&)and then a call to y(int&&). Output: 12.

Note [1]: §[dcl.ref]¶6 in the standard: "If a typedef-name (§10.1.3, §17.1) or a decltype-specifier (§10.1.7.2) denotes a type TR that is a reference to a type T, an attempt to create the type “lvalue reference to cv TR” creates the type “lvalue reference to T”, while an attempt to create the type “rvalue reference to cv TR” creates the type TR." The example at the end of that paragraph is worth a look.

Note from the contributor: This demonstrates Scott Meyers's advice to use std::forward for forwarding references, and std::move for rvalue references.

You can explore this question further on C++ Insights or Compiler Explorer!

https://cppinsights.io/lnk?code=I2luY2x1ZGUgPGlvc3RyZWFtPgojaW5jbHVkZSA8dXRpbGl0eT4KCmludCB5KGludCAmKSB7IHJldHVybiAxOyB9CmludCB5KGludCAmJikgeyByZXR1cm4gMjsgfQoKdGVtcGxhdGUgPGNsYXNzIFQ+IGludCBmKFQgJiZ4KSB7IHJldHVybiB5KHgpOyB9CnRlbXBsYXRlIDxjbGFzcyBUPiBpbnQgZyhUICYmeCkgeyByZXR1cm4geShzdGQ6Om1vdmUoeCkpOyB9CnRlbXBsYXRlIDxjbGFzcyBUPiBpbnQgaChUICYmeCkgeyByZXR1cm4geShzdGQ6OmZvcndhcmQ8VD4oeCkpOyB9CgppbnQgbWFpbigpIHsKICBpbnQgaSA9IDEwOwogIHN0ZDo6Y291dCA8PCBmKGkpIDw8IGYoMjApOwogIHN0ZDo6Y291dCA8PCBnKGkpIDw8IGcoMjApOwogIHN0ZDo6Y291dCA8PCBoKGkpIDw8IGgoMjApOwogIHJldHVybiAwOwp9&insightsOptions=cpp17&rev=1.0

https://godbolt.org/#%7B%22version%22%3A%204%2C%20%22content%22%3A%20%5B%7B%22type%22%3A%20%22row%22%2C%20%22content%22%3A%20%5B%7B%22type%22%3A%20%22component%22%2C%20%22componentName%22%3A%20%22codeEditor%22%2C%20%22componentState%22%3A%20%7B%22id%22%3A%201%2C%20%22source%22%3A%20%22%23include%20%3Ciostream%3E%5Cn%23include%20%3Cutility%3E%5Cn%5Cnint%20y%28int%20%26%29%20%7B%20return%201%3B%20%7D%5Cnint%20y%28int%20%26%26%29%20%7B%20return%202%3B%20%7D%5Cn%5Cntemplate%20%3Cclass%20T%3E%20int%20f%28T%20%26%26x%29%20%7B%20return%20y%28x%29%3B%20%7D%5Cntemplate%20%3Cclass%20T%3E%20int%20g%28T%20%26%26x%29%20%7B%20return%20y%28std%3A%3Amove%28x%29%29%3B%20%7D%5Cntemplate%20%3Cclass%20T%3E%20int%20h%28T%20%26%26x%29%20%7B%20return%20y%28std%3A%3Aforward%3CT%3E%28x%29%29%3B%20%7D%5Cn%5Cnint%20main%28%29%20%7B%5Cn%20%20int%20i%20%3D%2010%3B%5Cn%20%20std%3A%3Acout%20%3C%3C%20f%28i%29%20%3C%3C%20f%2820%29%3B%5Cn%20%20std%3A%3Acout%20%3C%3C%20g%28i%29%20%3C%3C%20g%2820%29%3B%5Cn%20%20std%3A%3Acout%20%3C%3C%20h%28i%29%20%3C%3C%20h%2820%29%3B%5Cn%20%20return%200%3B%5Cn%7D%22%2C%20%22options%22%3A%20%7B%22compileOnChange%22%3A%20true%2C%20%22colouriseAsm%22%3A%20true%7D%7D%7D%2C%20%7B%22type%22%3A%20%22column%22%2C%20%22content%22%3A%20%5B%7B%22type%22%3A%20%22component%22%2C%20%22componentName%22%3A%20%22compiler%22%2C%20%22componentState%22%3A%20%7B%22id%22%3A%201%2C%20%22source%22%3A%201%2C%20%22compiler%22%3A%20%22g92%22%2C%20%22filters%22%3A%20%7B%22b%22%3A%201%2C%20%22execute%22%3A%201%2C%20%22intel%22%3A%201%2C%20%22commentOnly%22%3A%201%2C%20%22directives%22%3A%201%7D%2C%20%22options%22%3A%20%22-std%3Dc%2B%2B17%22%7D%7D%2C%20%7B%22type%22%3A%20%22component%22%2C%20%22componentName%22%3A%20%22output%22%2C%20%22componentState%22%3A%20%7B%22compiler%22%3A%201%2C%20%22source%22%3A%201%7D%7D%5D%7D%5D%7D%5D%7D



*/