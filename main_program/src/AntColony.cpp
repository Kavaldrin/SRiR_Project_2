#include "AntColony.hpp"
#include <algorithm>
#include <numeric>
#include <algorithm>
#include <iostream>

using namespace AntColony;


void TSPSolver::_init()
{
    for(const auto& [pr, distance] : m_edges)
    {
        m_edgeDistances[pr] = distance;
        m_edgeWeights[pr] = STATING_WEIGHT;
    }

    m_tau0 = _computeTau0UsingNNAlgorithm();
    m_evolutionIterations = 100* PROBLEM_TYPE * m_nodesCount;

    m_gen = std::mt19937(std::random_device()());
    m_integerDistribution = std::uniform_int_distribution<>(0, m_nodesCount - 1);
    m_realDistribution = std::uniform_real_distribution<>(0.0, 1.0);
}

double TSPSolver::_computeTau0UsingNNAlgorithm()
{
    
    std::vector<int> nodesToVisit(m_nodesCount - 1);
    std::iota(nodesToVisit.begin(), nodesToVisit.end(), 1);
    int startingNode = 0;
    int currentNode = startingNode;
    double tourDistance = 0.0;

    while(!nodesToVisit.empty())
    {
        auto minNode = nodesToVisit.begin();
        double minVal = m_edgeDistances[{currentNode, *minNode}];

        for(auto nodeIt = minNode + 1; nodeIt != nodesToVisit.end(); ++nodeIt)
        {


            if(auto currentDistance = m_edgeDistances[{currentNode, *nodeIt}];
                currentDistance < minVal)
            {
                minNode = nodeIt;
                minVal = currentDistance;
            }
        }
        tourDistance += minVal;
        currentNode = *minNode;
        nodesToVisit.erase(minNode);
    }

    tourDistance += m_edgeDistances[{currentNode, startingNode}];
    return 1. / (m_nodesCount * tourDistance);
}


double TSPSolver::_getValueForEdge(const EdgeType_t& edge) const
{
    return m_edgeWeights.at(edge)
            * std::pow( 1. / m_edgeDistances.at(edge), BETA);
}

int TSPSolver::_computeNextCityForAgent(Agent& agent) const
{
    if(m_realDistribution(m_gen) < Q0)
    {
        auto bestFitNode = 0xFFFFFFFF;
        auto bestFitness = 0.0;
        for(auto& cityToVisit : agent.m_citiesToVisit)
        {
            if(auto current_val = _getValueForEdge({agent.m_currentCity, cityToVisit}); 
               current_val > bestFitness)
            {
                bestFitness = current_val;
                bestFitNode = cityToVisit;
            }
        }
        return bestFitNode;
    }
    else
    {
        auto fitnessSum = 0.0;
        for(auto& cityToVisit : agent.m_citiesToVisit)
        {
            fitnessSum += _getValueForEdge({agent.m_currentCity, cityToVisit});
        }

        auto givenProbability = m_realDistribution(m_gen);
        auto currentProbability = 0.0;

        for(auto& cityToVisit : agent.m_citiesToVisit)
        {
            currentProbability += _getValueForEdge({agent.m_currentCity, cityToVisit});

            if(currentProbability > givenProbability)
            {
                return cityToVisit;
            }
        }
        return agent.m_citiesToVisit.back();

    }
}

void TSPSolver::_makeTourAndMakeLocalUpdate(Agent& agent, int nextCity)
{

    agent.m_tourDistance += m_edgeDistances[{agent.m_currentCity, nextCity}];
    agent.m_tourHistory.emplace_back(std::pair{agent.m_currentCity, nextCity});

    auto prev_value = m_edgeWeights[ { agent.m_currentCity, nextCity } ];
    m_edgeWeights[ { agent.m_currentCity, nextCity } ] = (1 - P) * prev_value + P * m_tau0;
    agent.m_currentCity = nextCity;

}

void TSPSolver::initializeColony()
{

    for (int k = 0; k < M; ++k)
    {
        std::vector<int> currentCitiesToVisit(m_nodesCount);
        std::iota(std::begin(currentCitiesToVisit), std::end(currentCitiesToVisit), 0);

        //last index after swap will be current city
        int current_city = m_integerDistribution(m_gen);
        currentCitiesToVisit.erase(currentCitiesToVisit.begin() + current_city);

        m_agents.emplace_back( Agent{std::move(currentCitiesToVisit), current_city} );
    }

}

void TSPSolver::makeTours()
{

    //prepare agents

    for(auto& agent : m_agents)
    {
        agent.m_citiesToVisit = agent.m_citiesToVisitConst;
        agent.m_currentCity = agent.m_startingCity;
        agent.m_tourDistance = 0.0;
        agent.m_tourHistory.clear();
    }

    for(int i = 0; i < m_nodesCount - 1; ++i)
    {
        for(auto& agent : m_agents)
        {
            auto nextCity = _computeNextCityForAgent(agent);
            agent.m_citiesToVisit.erase(std::remove_if(agent.m_citiesToVisit.begin(), agent.m_citiesToVisit.end(),
                [&nextCity](auto& city){ return nextCity == city; }));

            _makeTourAndMakeLocalUpdate(agent, nextCity);
        }
    }

    //go back to the starting cities
    for(auto& agent : m_agents)
    {
        agent.m_citiesToVisit.push_back(agent.m_startingCity);
        _makeTourAndMakeLocalUpdate(agent, agent.m_startingCity);
    }

}

void TSPSolver::updateWeights()
{
    auto bestAgent = std::min_element(m_agents.begin(), m_agents.end(), [](const Agent& lhs, const Agent& rhs)
        { return lhs.m_tourDistance < rhs.m_tourDistance; });
    
    for(const auto& [start_v, end_v] : bestAgent->m_tourHistory)
    {
        auto prev_value = m_edgeWeights[{start_v, end_v}];
        m_edgeWeights[{start_v, end_v}] = (1 - ALFA) * prev_value + ALFA * (bestAgent->m_tourDistance);
    }
}


std::pair< std::vector< int >, double> TSPSolver::getBestRouteWithDistance()
{

    auto bestAgent = std::min_element(m_agents.begin(), m_agents.end(), [](const Agent& lhs, const Agent& rhs)
        { return lhs.m_tourDistance < rhs.m_tourDistance; });

    std::vector<int> route;
    for(auto& [v_start, v_end] : bestAgent->m_tourHistory)
    {
        (void)v_end;
        route.push_back(v_start);
    }
    route.push_back(bestAgent->m_startingCity);
    return { std::move(route), bestAgent->m_tourDistance };
}
