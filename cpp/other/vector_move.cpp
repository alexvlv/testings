/*
$Id$

g++ vector_move.cpp -std=c++11 -o vector_move && ./vector_move

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

static void print_vector_info(const std::vector<int> &vector)
{
		std::cerr << "vector size: " << vector.size() << " data: " << std::hex << reinterpret_cast<const void *>(vector.data()) << std::endl;
		std::ostream_iterator<int> out_it(std::cerr, " ");
		std::copy(vector.begin(), vector.end(), out_it);
		std::cerr << std::endl;
}

static std::vector<int> return_vector()
{
	std::cerr <<  "local vector inside: " << std::endl;
	std::vector<int> inside {40,30,20,10};
	print_vector_info(inside);
	return inside;
}

int main()
{
	std::cerr << "User defined types and std::pair ostream overload test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;
	
	std::vector<int> src{4,3,2,1};
	std::cerr << "vector src: " << std::endl;
	print_vector_info(src);
	
	std::vector<int> dst_copy = src;
	std::cerr << "vector copy: " << std::endl;
	print_vector_info(dst_copy);
	
	std::vector<int> dst_constructed(src);
	std::cerr << "vector copy constructed: " << std::endl;
	print_vector_info(dst_constructed);

	
//	std::vector<int> dst_move(std::move(src));
	std::vector<int> dst_move(static_cast<std::vector<int> &&>(src));
	print_vector_info(dst_move);
		
	std::cerr <<  "vector src after move: " << std::endl;
	print_vector_info(src);
	
	std::vector<int> returned = return_vector();
	std::cerr <<  "returned vector: " << std::endl;
	print_vector_info(returned);

	return 0;
}
