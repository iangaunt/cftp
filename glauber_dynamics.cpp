#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <iostream>
#include <random>
#include "random_trees.h"
#include "dynamic_programming.h"
#include <cmath>
#include <boost/graph/copy.hpp>
#include <future>
#include <thread>

// Random step T_k in CFTP (vertex v with prob. u)
struct Step {
    int vertex;
    double u;
};

// Represents if a vertex is inside the 
// coalesced independent set or not
enum VertState { 
    IN, OUT, 
    UNKNOWN // UNKNOWN = not coalesced
};

bool is_independent_set(const std::set<int>& independent_set, int vertex, const Graph& g) {
    for (auto neighbor : boost::make_iterator_range(boost::adjacent_vertices(vertex, g))) {
        if (independent_set.find(neighbor) != independent_set.end()) {
            return false;
        }
    }
    return true;
}

std::set<int> glauber_dynamics(const Graph& g, int T, unsigned int seed, double lambda) {
    std::set<int> independent_set;
    std::mt19937 rng(seed);
    std::uniform_int_distribution<> vertex_picker(0, boost::num_vertices(g) - 1);
    std::uniform_real_distribution<> percent_chance(0.0, 1.0);

    for (int t = 0; t < T; t++) {
        int vertex = vertex_picker(rng);

        if (independent_set.find(vertex) != independent_set.end()) {
            if (percent_chance(rng) <  1 / (1 + lambda)) {
                independent_set.erase(vertex);
            }
        }
        else if (is_independent_set(independent_set, vertex, g)) {
            if (percent_chance(rng) < lambda / (1 + lambda)) {
                independent_set.insert(vertex);
            }
        }
    }

    return independent_set;
}

void step_forward(std::vector<int>& state, const Step& step, const Graph& g, double lambda) {
    int v = step.vertex;

    bool any_neighbors_in = false;
    bool all_neighbors_out = true;

    for (auto neighbor : boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
        if (state[neighbor] == VertState::IN) {
            any_neighbors_in = true;
        }

        if (state[neighbor] != VertState::OUT) {
            all_neighbors_out = false;
        }
    }

    const double insert_prob = lambda / (1.0 + lambda);
    const double remove_prob = 1.0 / (1.0 + lambda);

    if (step.u > insert_prob && step.u > remove_prob) {
        return;
    }

    if (step.u < insert_prob && step.u < remove_prob) {
        if (any_neighbors_in || state[v] == VertState::IN) {
            state[v] = VertState::OUT;
        } else if (state[v] == VertState::OUT && all_neighbors_out) {
            state[v] = VertState::IN;
        } else {
            state[v] = VertState::UNKNOWN;
        }

        return;
    }

    // If one of the events occurs, then check to see 
    // if it favors insertions (high lambda) or removals (low lambda)
    if (lambda > 1.0) {
        if (any_neighbors_in) {
            state[v] = VertState::OUT;
        } else if (all_neighbors_out) {
            state[v] = VertState::IN;
        } else {
            state[v] = VertState::UNKNOWN;
        }
    } else {
        state[v] = VertState::OUT;
    }
}

std::set<int> glauber_cftp(const Graph& g, unsigned int seed, double lambda) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<> vertex_picker(0, boost::num_vertices(g) - 1);
    std::uniform_real_distribution<> percent_chance(0.0, 1.0);

    const int n = static_cast<int>(boost::num_vertices(g));
    if (n == 0) {
        return {};
    }

    std::deque<Step> steps;
    for (int T = 1; ; T++) {
        //printf("n: %d, T: %d\n", n, T);

        while (steps.size() < T) {
            steps.push_front(
                {vertex_picker(rng), percent_chance(rng)}
            );
        }

        std::vector<int> state(n, UNKNOWN);
        for (Step& step : steps) {
            step_forward(state, step, g, lambda);
        }

        bool coalesced = true;
        for (auto s : state) {
            if (s == UNKNOWN) { 
                coalesced = false; 
                break; 
            }
        }

        if (coalesced) {
            std::set<int> independent_set;
            for (int v = 0; v < n; v++) {
                if (state[v] == IN) {
                    independent_set.insert(v);
                }
            }

            std::cout << "n: " << n << ", coalescence time: " << T << std::endl;
            return independent_set;
        }
    }
}