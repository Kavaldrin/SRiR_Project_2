#ifndef __IO_MANAGER_HPP__
#define __IO_MANAGER_HPP__

#include <fstream>
#include <vector>
#include "cpp_utils.hpp"

class IOManager
{
    NON_INSTANTIATION_CLASS(IOManager)
public:
    static std::pair< std::vector< std::pair<std::pair<int, int>, double >>, int> 
        getEdgesWithDistances(const std::string& filename);

};

#endif