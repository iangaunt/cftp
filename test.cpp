#include "random_trees.h"
#include "dynamic_programming.h"
#include "glauber_dynamics.h"
#include <thread>

#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int num_processes, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int epochs = 5;
    int color = rank % epochs;
    MPI_Comm sub_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &sub_comm);

    int sub_rank, sub_size;
    MPI_Comm_rank(sub_comm, &sub_rank);
    MPI_Comm_size(sub_comm, &sub_size);

    Graph g;
    double alpha = 1;
    int T = 1;
    double K = 200;
    double lambda = 1;
    int n = 10;
    std::random_device rd;


    // For a concrete example of how my distribution works, let's say I have 5 epochs, 8 processes, and 17 samples.
    // The distribution will first allocate as many processes as it can for each epoch using MPI_Comm_split. In our example, 3 epochs get 2 processes each, and 2 epochs get 1 process each.
    // Then, for each epoch, we want to do 17 samples. For each epoch with 2 processes each, one process will do 9 samples, and the other will do 8. For each epoch with 1 process each, each process will do 17 samples.
    // This way, we can isolate epochs into individual communication groups, and come up with an alpha estimate for each epoch. Then, we can sum up those estimates and average them out.

    // The completion time is bottlenecked by the process with the highest number of samples, so ideally have the number of processes be evenly divisible by the number of epochs.

    // This is the number of processes allocated to a given epoch
    int num_processes_for_epoch;
    MPI_Comm_size(sub_comm, &num_processes_for_epoch);

    // Given the number of processes for this epoch, this calculates the number of samples for each process.
    int samples = K;
    int samples_per_process = samples / num_processes_for_epoch;
    int extra_samples = samples % num_processes_for_epoch;
    if (sub_rank < extra_samples) {
        samples_per_process++;
    }

    if (rank == 0) {
        std::cout << "Lambda = " << lambda << " K = " << K << std::endl;
    }

    std::cout << "Samples per process: " << samples_per_process << std::endl;

    for (n = 2; n < 1024; n *= 2) {
        std::vector<int> prufer_sequence = create_prufer_sequence(n, 12781358);
        g = prufer_sequence_to_tree(prufer_sequence);
        auto edges = boost::edges(g);
        std::vector<Graph::edge_descriptor> edges_list(edges.first, edges.second);
        int m = boost::num_edges(g);


        double actual_independent_sets = 0;
        if (rank == 0) {
            actual_independent_sets = num_ind_sets(g, 0, lambda);
            std::cout << "\nStarting simulation for n = " << n << std::endl;
            std::cout << "Actual Number of Independent Sets: " << actual_independent_sets << std::endl;
        }
        MPI_Bcast(&actual_independent_sets, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        bool within_threshold = false;

        T = 1;

        while (!within_threshold) {
            // We need to choose a specific seed so that all cores (which don't share memory) have access to the same graph.
            g = prufer_sequence_to_tree(prufer_sequence);
            auto edges = boost::edges(g);
            std::vector<Graph::edge_descriptor> edges_list(edges.first, edges.second);
            int m = boost::num_edges(g);

            double sum_alpha = 0;
            alpha = 1;
            for (int i = 0; i < m; i++) {
                auto edge = edges_list[i];
                int removed_edge_source = boost::source(edge, g);
                int removed_edge_target = boost::target(edge, g);


                boost::remove_edge(edge, g);
                // If this is the last edge (the Mth edge), alpha is 2^n
                if (i == edges_list.size() - 1 && sub_rank == 0) {
                    alpha *= pow(2, num_vertices(g));
                }
                else {

                    double local_invalid_sets = 0;
                    for (int s = 0; s < samples_per_process; s++) {
                        std::set<int> sample = glauber_dynamics(g, T, rd(), lambda);
                        if (sample.find(removed_edge_source) != sample.end() && sample.find(removed_edge_target) != sample.end()) {
                            local_invalid_sets++;
                        }
                    }

                    double total_invalid_sets = 0;
                    MPI_Reduce(&local_invalid_sets, &total_invalid_sets, 1, MPI_DOUBLE, MPI_SUM, 0, sub_comm);

                    if (sub_rank == 0) {
                        // The ratio that I multiply alpha by is the number of sets I sampled from Gi+1 that were also independent sets in Gi over the total number of sets I sampled.
                        alpha *= (K - total_invalid_sets) / K;
                    }
                    else {
                        alpha = 0;
                    }
                }
            }

            std::cout << "World Rank: " << rank << " Sub Rank: " << sub_rank << " Alpha: " << alpha << std::endl;
            MPI_Reduce(&alpha, &sum_alpha, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

            if (rank == 0) {
                double averaged_alpha = sum_alpha / (num_processes < epochs ? num_processes : epochs);

                if (averaged_alpha <= actual_independent_sets * 1.4) {
                    std::cout << "Averaged Alpha: " << averaged_alpha  << std::endl;
                    std::cout << "T: " << T << std::endl;
                    std::cout << T << "," << n << std::endl;
                    within_threshold = true;
                }
            }

            MPI_Bcast(&within_threshold, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
            T *= 2;

        }
    }

    MPI_Comm_free(&sub_comm);
    MPI_Finalize();
    return 0;
}
