#include "random_trees.h"
#include "glauber_dynamics.h"
#include <thread>
#include <fstream>

#include <mpi.h>

#define LAMBDA 1.6
#define RUNS 25

int main(int argc, char *argv[]) {
    Graph g;
    std::random_device rd;

    std::ofstream file{"data.csv"};
	file << "n,log2(T)\n";

    for (int n = 50; n < INT_MAX; n += 50) {
        std::vector<int> prufer_sequence = create_prufer_sequence(n, 12781358);
        g = prufer_sequence_to_tree(prufer_sequence);
        auto edges = boost::edges(g);
        std::vector<Graph::edge_descriptor> edges_list(edges.first, edges.second);

        float T_sum = 0;
        for (int i = 0; i < RUNS; i++) {
            T_sum += std::log2(glauber_cftp(g, rd(), LAMBDA));
        }
        file << n << "," << T_sum / RUNS << std::endl;
    }

    return 0;
}
