#include "random_trees.h"
#include "dynamic_programming.h"
#include "glauber_dynamics.h"
#include <thread>

#include <mpi.h>

int main(int argc, char *argv[]) {
    Graph g;
    double lambda = 0.5;
    std::random_device rd;

    for (int n = 10; n < INT_MAX; n += 10) {
        std::vector<int> prufer_sequence = create_prufer_sequence(n, 12781358);
        g = prufer_sequence_to_tree(prufer_sequence);
        auto edges = boost::edges(g);

        std::vector<Graph::edge_descriptor> edges_list(edges.first, edges.second);
        glauber_cftp(g, rd(), lambda);
    }

    return 0;
}
