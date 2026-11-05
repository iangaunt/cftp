//
// Created by Shayan Daijavad on 7/29/24.
//

#ifndef SURP2024_GLAUBER_DYNAMICS_H
#define SURP2024_GLAUBER_DYNAMICS_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <vector>
#include <iostream>
#include <random>


typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;

bool is_independent_set(const std::set<int>& independent_set, int vertex, const Graph& g);
std::set<int> glauber_dynamics(const Graph& g, int T, unsigned int seed, double lambda);
std::set<int> glauber_cftp(const Graph& g, unsigned int seed, double lambda);

#endif //SURP2024_GLAUBER_DYNAMICS_H
