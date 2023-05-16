#include "closest_point.h"

//gcc benchmark_test.c util.c nlogn_sol.c quadratic_sol.c multiproc_sol.c multithread_sol.c -o out/benchmark_test
int main(int argc, char const *argv[])
{
    int num_points;
    struct pointlist pd;

    if (argc < 2)
    {
        printf("USAGE: %s test_mode [sample_size|filename]", argv[0]);
        return EXIT_FAILURE;
    }

    const char* test_mode = argv[1];
    if(strcmp(test_mode, "random") == 0) {
        if(argc < 3) {
            printf("Usage: %s 'random' sample_size", argv[0]);
            return EXIT_FAILURE;
        }
        num_points = atoi(argv[2]);
        point* P = rand_points(num_points);
        pd.num_points = num_points;
        pd.P = P;
    }
    else if(strcmp(test_mode, "file") == 0) {
        if(argc < 3) {
            printf("Usage: %s 'file' filename", argv[0]);
            return EXIT_FAILURE;
        }
        pd = read_test_file(argv[2]);
    }
    else{
        printf("test mode must be one of: 'random', 'file'");
        return EXIT_FAILURE;
    }

    printf("nlogn_solution: num_points=%ld, num_processes=%d, time=%.4f seconds\n", pd.num_points, -1, timeit(nlogn_solution, pd, -1));
    printf("nlogn_solution_multiproc: num_points=%ld, num_processes=%d, time=%.4f seconds\n", pd.num_points, 2, timeit(nlogn_solution_multiproc, pd, 8));
    printf("nlogn_solution_multithread: num_points=%ld, num_processes=%d, time=%.4f seconds\n", pd.num_points, 2, timeit(nlogn_solution_multithread, pd, 8));
    printf("quadratic_solution: num_points=%ld, num_processes=%d, time=%.4f seconds\n", pd.num_points, -1, timeit(quadratic_solution, pd, -1));

    return EXIT_SUCCESS;
}
