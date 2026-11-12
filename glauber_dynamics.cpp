#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <iostream>
#include <random>
#include "random_trees.h"
#include <boost/graph/copy.hpp>
#include <queue>

#define IN 1
#define OUT 0

// Random step T_k in CFTP (vertex v with prob. u)
struct Step {
    int vertex;
    double u;
};

void step_forward(std::vector<int>& state, const Step& step, const Graph& g, double lambda) {
    int v = step.vertex;
    bool all_neighbors_out = true;

    for (auto neighbor : boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
        if (state[neighbor] == IN) {
            all_neighbors_out = false;
            break;
        }
    }

    // Remove vertex if v is in the set
    if (state[v] == 1 && step.u < 1.0 / (1.0 + lambda)) {
        state[v] = OUT;
        return;
    }
    
    // Add vertex if v is not in the set 
    // and no neighbors are in the set
    if (state[v] == 0 && step.u < lambda / (1.0 + lambda) && all_neighbors_out) {
        state[v] = IN;
        return;
    }
}

// Calculates depth of each vertex in the graph from
// the first vertex (supposed root)
void calculate_depth(const Graph& g, std::vector<int>& depth) {
    std::queue<int> q;
    depth[0] = 0;
    q.push(0);

    while (!q.empty()) {
        int vertex = q.front();
        q.pop();

        for (auto neighbor : boost::make_iterator_range(boost::adjacent_vertices(vertex, g))) {
            if (depth[neighbor] == -1) {
                depth[neighbor] = depth[vertex] + 1;
                q.push(neighbor);
            }
        }
    }
}

int glauber_cftp(const Graph& g, unsigned int seed, double lambda) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<> vertex_picker(0, boost::num_vertices(g) - 1);
    std::uniform_real_distribution<> percent_chance(0.0, 1.0);

    const int n = static_cast<int>(boost::num_vertices(g));
    if (n == 0) {
        return {};
    }

    // Create min and max states (alternating IN and OUT for each generation)
    std::vector<int> depth(n, -1);
    calculate_depth(g, depth);

    std::vector<int> min_tree(n);
    std::vector<int> max_tree(n);
    std::deque<Step> steps;
    
    for (int T = 1; ; T *= 2) {
        while (steps.size() < T) {
            steps.push_front(
                {vertex_picker(rng), percent_chance(rng)}
            );
        }

        for (int i = 0; i < n; i++) {
            min_tree[i] = depth[i] % 2 == 0 ? OUT : IN;
            max_tree[i] = depth[i] % 2 == 0 ? IN : OUT;
        }

        // Perform coupling from the past on both max & min states
        for (Step& step : steps) {
            step_forward(min_tree, step, g, lambda);
            step_forward(max_tree, step, g, lambda);
        }

        bool coalesced = true;
        for (int i = 0; i < n; i++) {
            if (min_tree[i] != max_tree[i]) {
                coalesced = false;
                break;
            }
        }

        if (coalesced) {
            return T;
        }
    }
}
