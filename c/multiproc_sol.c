#include "closest_point.h"

/**
 * 
 * Calculate the number of points recursively by splitting the work and starting a new process on each recursion
 * Once the level of parallelism is reached, no more processes are created and the remaining work is delegated to the
 * sequential version of this function
 * 
 * The result is stored in one of the variables passed as arguments (that may be a region in the heap if the call is made
 * within the same process or a memory mapping if the call is made on a new process)
 * 
 * Note: a child process created by fork() inherits copies of its parent's mappings that refer to the same pages of
 * physical memory
 * 
 */
void closest_points_multiproc(point Px[], PyElement Py[], size_t length, points_distance *result, int par_threshold)
{
    if (length == 2)
    { // base case
        result->p1 = Px[0];
        result->p2 = Px[1];
        result->distance = distance(result->p1, result->p2);
    }
    else if (length > par_threshold)
    {
        // if the number of points is big enough, the work to compute each half is split
        // between the current process (right half) and a new one (left half)
        size_t left_half_upper_bound = ceil(length / 2.);
        size_t right_half_lower_bound = floor(length / 2.);
        size_t left_size = left_half_upper_bound;
        size_t right_size = length - right_half_lower_bound;
        PyElement *Ly = (PyElement *)malloc(left_size * sizeof(PyElement));
        if (Ly == NULL)
            errExit("malloc Ly");
        PyElement *Ry = (PyElement *)malloc(right_size * sizeof(PyElement));
        if (Ry == NULL)
            errExit("malloc Ry");

        populateLy(Py, length, Ly, left_half_upper_bound);
        populateRy(Py, length, Ry, right_half_lower_bound);

        points_distance *left_result = mmap(NULL, sizeof(points_distance), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (left_result == MAP_FAILED)
            errExit("mmap");
        points_distance *right_result = (points_distance *)malloc(sizeof(points_distance));
        if (right_result == NULL)
            errExit("malloc right_result");

        pid_t pid = fork();
        switch (pid)
        {
        case -1: //error
            errExit("fork");
        case 0: //child: works the left half
            closest_points_multiproc(Px, Ly, left_size, left_result, par_threshold);
            _exit(EXIT_SUCCESS); // we need to exit here to stop the execution of the inherited stack frames
        default: //parent: works the right half
            closest_points_multiproc(Px + right_half_lower_bound, Ry, right_size, right_result, par_threshold);
            pid_t child_pid;
            do
            {
                child_pid = waitpid(pid, NULL, 0);
            } while (child_pid == -1 && errno == EINTR);
            if (child_pid == -1 && errno != ECHILD)
                errExit("error while waiting");

            float min_left_distance = left_result->distance;
            float min_right_distance = right_result->distance;

            float min_distance_upper_bound = MIN(min_left_distance, min_right_distance);
            size_t *candidates_length = (size_t *)malloc(sizeof(size_t));
            if (candidates_length == NULL)
                errExit("malloc candidates_length");

            PyElement *candidates = get_candidates_from_different_halves(*(Px + left_size - 1), Py, length, candidates_length, min_distance_upper_bound);
            points_distance closest_candidates = closest_points_from_different_halves(candidates, *candidates_length);

            free(candidates_length);
            free(Ly);
            free(Ry);
            free(candidates);

            if (closest_candidates.distance < min_distance_upper_bound)
            {
                result->p1 = closest_candidates.p1;
                result->p2 = closest_candidates.p2;
                result->distance = closest_candidates.distance;
            }
            else if (min_left_distance < min_right_distance)
            {
                result->p1 = left_result->p1;
                result->p2 = left_result->p2;
                result->distance = left_result->distance;
            }
            else
            {
                result->p1 = right_result->p1;
                result->p2 = right_result->p2;
                result->distance = right_result->distance;
            }
            free(right_result);
            munmap(left_result, sizeof(points_distance));
        }
    }
    else
    {
        // if the number of points is small enough, all the work is done by the current process
        points_distance intermediate_result = closest_points(Px, Py, length);
        result->p1 = intermediate_result.p1;
        result->p2 = intermediate_result.p2;
        result->distance = intermediate_result.distance;
    }
}

points_distance nlogn_solution_multiproc(point P[], size_t length, int num_processes)
{
    assert(length >= 2);
    int par_threshold = ceil((float)length / num_processes);
    points_distance *ptr_result = (points_distance *)malloc(sizeof(points_distance));
    PyElement *Py = (PyElement *)malloc(length * sizeof(PyElement));
    sort_points(P, length, Py);
    closest_points_multiproc(P, Py, length, ptr_result, par_threshold);
    free(Py);
    points_distance result = *ptr_result;
    free(ptr_result);
    return result;
}

#ifdef FAB_MAIN
int main(int argc, char const *argv[])
{
    perf_test_random(nlogn_solution_multiproc, 10000000, 8);
}
#endif

//num_points=1000, num_processes=8, time=0.0033 seconds
//num_points=10000, num_processes=8, time=0.0097 seconds
//num_points=100000, num_processes=8, time=0.0999 seconds
//num_points=1000000, num_processes=8, time=1.1675 seconds
//num_points=10000000, num_processes=8, time=15.4827 seconds