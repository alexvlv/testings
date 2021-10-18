#include <iostream>
#include <string>
#include <vector>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    for (auto& el : vec)
    {
        os << el << ' ';
    }
    return os;
}

int main()
{
    std::vector<std::string> vec = {
        "Hello", "from", "GCC", __VERSION__, "!" 
    };
    
    std::vector<std::string> &vecref = vec;
    std::vector<std::string> &vecrefref = vecref;
    std::vector<std::string> &vecref3 = vecrefref;
    
    
    std::vector<std::string> *vecptr = &vec;
    std::vector<std::string> *vecptr3 = &vecrefref;
    
    std::cout << vec << std::endl;
    std::cerr << reinterpret_cast<void *>(&vec) << " " <<  reinterpret_cast<void *>(&vecref) << " " <<  reinterpret_cast<void *>(&vecrefref) << " " <<  reinterpret_cast<void *>(&vecref3) << std::endl;
    std::cerr << reinterpret_cast<void *>(vecptr) << " " <<  reinterpret_cast<void *>(vecptr3) << std::endl;
}

// https://coliru.stacked-crooked.com/a/c67d4ff69cddd1c4

