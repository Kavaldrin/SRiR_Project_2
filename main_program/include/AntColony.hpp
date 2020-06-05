
#ifndef __ANT_COLONY_HPP__
#define __ANT_COLONY_HPP__

#include <utility>
#include <vector>
#include <unordered_map>
#include <random>
#include <memory>

namespace AntColony
{


using EdgeType_t = std::pair<int, int>;
using EdgeTypeWithValue_t = std::pair<EdgeType_t, double>;



class TSPSolver
{
    
public:

    TSPSolver(int nodesCount, std::vector<EdgeTypeWithValue_t>&& edges)
        :m_nodesCount(nodesCount), m_edges(edges) { _init(); }
    TSPSolver(int nodesCount, std::vector<EdgeTypeWithValue_t>& edges)
        :m_nodesCount(nodesCount), m_edges(edges) { _init(); }

    void initializeColony();
    void makeTours();
    void updateWeights();
    unsigned int getMaxEvolutionIterations() const { return m_evolutionIterations; }
    std::pair< std::vector< int >, double> getBestRouteWithDistance();

private:

    struct Agent
    {

        Agent(const std::vector<int>& cities, int startingCity):m_citiesToVisitConst(cities), 
            m_startingCity(startingCity){}

        Agent(std::vector<int>&& cities, int startingCity):m_citiesToVisitConst(cities), 
            m_startingCity(startingCity){}
        
        const std::vector<int> m_citiesToVisitConst;
        const int m_startingCity;

        std::vector<int> m_citiesToVisit;
        int m_currentCity;
        double m_tourDistance = 0.0;
        std::vector< EdgeType_t > m_tourHistory;

    };

    struct EdgeHash
    {
        std::size_t operator()(const std::pair<int, int>& pr) const noexcept
        { return std::hash<int>()(pr.first) ^ std::hash<int>()(pr.second); }
    };


private:

    void _init();
    double _computeTau0UsingNNAlgorithm();
    int _computeNextCityForAgent(Agent& agent) const;
    double _getValueForEdge(const EdgeType_t& edge) const;
    void _makeTourAndMakeLocalUpdate(Agent& agent, int nextCity);


private:
    const int m_nodesCount;
    const std::vector<EdgeTypeWithValue_t> m_edges;

    unsigned int m_evolutionIterations;
    double m_tau0;
    std::unordered_map<EdgeType_t, double, EdgeHash> m_edgeDistances;
    std::unordered_map<EdgeType_t, double, EdgeHash> m_edgeWeights;
    std::vector<Agent> m_agents;

    mutable std::mt19937 m_gen;
    mutable std::uniform_int_distribution<> m_integerDistribution;
    mutable std::uniform_real_distribution<> m_realDistribution; 


public:

    //Number of ants in colony
    static constexpr int M = 10;
    //Probability of using normal formula to compute next city
    static constexpr double Q0 = 0.9;
    //Starting weight of each edges
    static constexpr double STATING_WEIGHT = 1.;
    static constexpr int PROBLEM_TYPE = 200;

    //Consts needed by algorithm
    static constexpr int BETA = 2;
    static constexpr double P = 0.1;
    static constexpr double ALFA = 0.1;

};




}


#endif
