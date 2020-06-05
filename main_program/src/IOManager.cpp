#include "IOManager.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <iostream>


std::pair< std::vector< std::pair<std::pair<int, int>, double >>, int> IOManager::getEdgesWithDistances(const std::string& filename)
{

    std::vector< std::pair<std::pair<int, int>, double >> edges;

    std::ifstream file(filename, std::ifstream::in);
    if(! file.is_open()){ throw std::runtime_error("Cannot open file!"); }

    std::stringstream ss;
    std::copy(std::istream_iterator<char>(file), std::istream_iterator<char>(), std::ostream_iterator<char>(ss));

    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);

    int maxNodeID = 0;

    for(auto it = pt.begin(); it != pt.end(); ++it)
    {
        int currentNode = it->second.get<int>("id");
        if(currentNode > maxNodeID){ maxNodeID = currentNode; }

        for(auto& adjacency : it->second.get_child("adjacencies"))
        {
            edges.emplace_back(
                std::pair{currentNode, adjacency.second.get<int>("nodeTo")}, 
                adjacency.second.get<double>("data.weight"));
        }
    }

    file.close();

    return {edges, maxNodeID + 1};
}
