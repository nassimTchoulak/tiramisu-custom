#include "Halide.h"
#include "wrapper.h"   // declares function_jacobi1d_XLARGE and the helpers

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

#define SIZE 4000      // must match the buffer size in generator.cpp

static double median(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    return (n % 2) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

int main(int, char **)
{
    declare_memory_usage();

    // Same shapes and order as the buffers given to tiramisu::codegen()
    Halide::Buffer<double> b_A(SIZE), b_B(SIZE);

    // Number of runs is driven by environment variables (see wrapper.h):
    //   MAX_RUNS, DYNAMIC_RUNS (0/1) and, if dynamic, MIN_RUNS and INIT_EXEC_TIME
    int nb_exec = get_max_nb_runs();
    const bool dynamic = is_nb_runs_dynamic();
    const int min_runs = dynamic ? get_min_nb_runs() : nb_exec;

    std::vector<double> durations;   // milliseconds

    for (int run = 0; run < nb_exec; run++)
    {
        // Re-initialise every run: the kernel updates the buffers in place
        parallel_init_buffer(b_A.data(), SIZE, 1.0);
        parallel_init_buffer(b_B.data(), SIZE, 1.0);

        auto start = std::chrono::steady_clock::now();
        function_jacobi1d_XLARGE(b_A.raw_buffer(), b_B.raw_buffer());
        auto end = std::chrono::steady_clock::now();

        durations.push_back(std::chrono::duration<double, std::milli>(end - start).count());

        // Once MIN_RUNS are done, decide how many more are really needed
        if (dynamic && (int) durations.size() == min_runs)
            nb_exec = std::min(nb_exec, (int) durations.size() + choose_nb_runs(durations));
    }

    for (double d : durations)
        std::cout << d << " ";
    std::cout << std::endl;
    std::cout << "median: " << median(durations) << " ms" << std::endl;

    return 0;
}