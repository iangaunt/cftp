

#ifndef RANDOM_TREES_H
#define RANDOM_TREES_H

#include <boost/graph/adjacency_list.hpp>
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> Graph;


std::vector<int> create_prufer_sequence(int num_vertices, unsigned int seed);
void print_prufer_sequence(const std::vector<int>& prufer_sequence);
Graph prufer_sequence_to_tree(const std::vector<int>& prufer_sequence);
void tree_to_dot(const Graph& g, const std::string& filename);

#endif //RANDOM_TREES_H
