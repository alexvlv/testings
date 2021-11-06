/*
$Id$

g++ std_pair_ostream_overload.cpp -std=c++11 -o ostream && ./ostream

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


struct Rec 
{
	int a;
	int b;
	int c;
};

static std::ostream& operator<<(std::ostream& os, const Rec &rec)
{
	os << rec.a << "#" << rec.b;
	return os;
}

using Pair = std::pair<int, int>;
//typedef std::pair<int, int> Pair;
namespace std {
	static std::ostream& operator<<(std::ostream& os, const Pair &pair)
	{
		os << pair.first << "%" << pair.second;
		return os;
	}
}

static std::string pair2string(const Pair &pair)
{
	std::ostringstream str;
	str << pair.first << "&" << pair.second;
	return str.str();
}

int main()
{
	std::cerr << "User defined types and std::pair ostream overload test" << std::endl
		<< "$Id$" << std::endl
		<<"Compiled: " __DATE__ " " __TIME__ << std::endl;

	std::vector<Rec> recs { {0,},{1,},{2,},{3,},{4,},{5,},};
	std::ostream_iterator<Rec> rec_out_it (std::cout,", ");
	std::copy(recs.begin(),recs.end(),rec_out_it);
	std::cout << std::endl;

	std::vector<Pair> pairs { {0,0},{1,-1},{2,-2},{3,-3},{4,-4},{5,-5}, };
	std::ostream_iterator<Pair> pair_out_it (std::cout,", ");
	std::copy(pairs.begin(),pairs.end(),pair_out_it);

	auto f92s = [](const Pair &pair) {
		std::ostringstream str;
		str << pair.first << "#" << pair.second;
		return str.str();
	};
	std::cout << std::endl;
	
	std::transform(pairs.begin(),pairs.end(),
		std::ostream_iterator<std::string>(std::cout,", "),
		f92s
		//pair2string
		);
	std::cout << std::endl;
	
	return 0;
}
//-------------------------------------------------------------------------
