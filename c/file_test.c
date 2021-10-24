#include "closest_point.h"

//gcc file_test.c util.c nlogn_sol.c quadratic_sol.c multiproc_sol.c multithread_sol.c -o out/file_test
int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("USAGE: filename [num_points]");
        return EXIT_FAILURE;
    }

    struct pointlist pd = read_test_file(argv[1]);
    points_distance solution = nlogn_solution(pd.P, pd.num_points, -1);
    print_points_distance(solution);
    assert(quadratic_solution(pd.P, pd.num_points, -1).distance == solution.distance);
    assert(nlogn_solution_multiproc(pd.P, pd.num_points, 2).distance == solution.distance);
    assert(nlogn_solution_multithread(pd.P, pd.num_points, 2).distance == solution.distance);
    return EXIT_SUCCESS;
}