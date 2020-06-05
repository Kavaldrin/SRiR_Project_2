#include <iostream>
#include "AntColony.hpp"
#include "IOManager.hpp"
#include <algorithm>
#include <chrono>
#include <type_traits>
#include <upcxx/upcxx.hpp>


int main(int argc, char** argv)
{

    constexpr int masterID = 0;


    upcxx::init();
    const int currentMachineID = upcxx::rank_me();
    const int processorsNum = upcxx::rank_n();
    
    decltype(std::chrono::steady_clock::now()) time_start;
    if(masterID == currentMachineID)
    {
        time_start = std::chrono::steady_clock::now();
    }

    std::vector<AntColony::EdgeTypeWithValue_t > edges;

    int nodesCount = 0, edgesCount = 0;

    if(masterID == currentMachineID)
    {
        std::string filename = "generated_graph.g";
        if(argc == 2){ filename = argv[1]; }
        auto [edges_, nodesCount_] = IOManager::getEdgesWithDistances(filename);
        
        edges = std::move(edges_);

        nodesCount = nodesCount_;
        edgesCount = edges.size();
    }

    nodesCount = upcxx::broadcast(nodesCount, masterID).wait();
    edgesCount = upcxx::broadcast(edgesCount, masterID).wait();

    if(masterID != currentMachineID)
    {
        edges.resize(edgesCount);
    }

    upcxx::broadcast(edges.data(), edges.size(), masterID).wait();

    AntColony::TSPSolver antColony(nodesCount, std::move(edges));
    antColony.initializeColony();
    for(unsigned evolution = 0; evolution < antColony.getMaxEvolutionIterations(); ++evolution)
    {
        antColony.makeTours();
        antColony.updateWeights();
    }
    
    upcxx::barrier();
    std::vector< upcxx::global_ptr<int> > bestRoutes(processorsNum);
    std::vector< upcxx::global_ptr<double> > bestDistances(processorsNum);

    if(masterID == currentMachineID)
    {
        for(auto& globalPtr : bestRoutes) { globalPtr = upcxx::new_array<int>(nodesCount + 1); }
        for(auto& globalPtr : bestDistances) { globalPtr = upcxx::new_<double>(nodesCount); }
    }


    upcxx::broadcast(bestRoutes.data(), bestRoutes.size(), masterID).wait();
    upcxx::broadcast(bestDistances.data(), bestDistances.size(), masterID).wait();

    auto [bestRoute, bestDistance] = antColony.getBestRouteWithDistance();
    upcxx::rput(bestRoute.data(), bestRoutes[currentMachineID], bestRoute.size());
    upcxx::rput(bestDistance, bestDistances[currentMachineID]);
    
    upcxx::barrier();

    if(masterID == currentMachineID)
    {     

        std::vector< std::pair < std::vector < int >, double > > routes(processorsNum);
        for(int idx = 0; idx < processorsNum; ++idx)
        {
            routes[idx].first.resize(nodesCount + 1);
            upcxx::rget(bestRoutes[idx], routes[idx].first.data(), nodesCount + 1).wait();
            auto distanceRaw = upcxx::rget(bestDistances[idx]).wait();
            routes[idx].second = distanceRaw;
        }
       

        auto bestRoute = std::min_element(routes.begin(), routes.end(), 
            [](const std::pair<std::vector<int>, double>& lhs, const std::pair<std::vector<int>, double>& rhs)
            {
                return lhs.second < rhs.second;
            });

        std::cout << "Route: ";
        for(const auto& currentNode : bestRoute->first)
        {
            std::cout << currentNode << " ";
        }
        std::cout << "\nDistance: " << bestRoute->second << std::endl;

        std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - time_start).count() << "ms" << std::endl;

    }

    upcxx::finalize();

    return 0;
}