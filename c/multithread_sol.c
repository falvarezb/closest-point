#include "closest_point.h"

/**
 * 
 * Calculate the number of points recursively by splitting the work and starting a new thread on each recursion
 * Once the level of parallelism is reached, no more threads are created and the remaining work is delegated to the
 * serial version of this function
 * 
 * To be able to call this function recursively in a new thread, we need to adopt the signature
 * expected by 'pthread_create'
 * 
 * The result is stored in one of the variables passed as arguments
 */
void *closest_points_multithread(void *targs)
{
    thread_arg *args = (thread_arg *)targs;
    if (args->length == 2)
    { // base case
        args->result->p1 = args->Px[0];
        args->result->p2 = args->Px[1];
        args->result->distance = distance(args->result->p1, args->result->p2);
    }
    else if (args->length > args->par_threshold)
    {
        // if the number of points is big enough, the work to compute each half is split
        // between the current thread (right half) and a new one (left half)
        size_t left_half_upper_bound = ceil(args->length / 2.);
        size_t right_half_lower_bound = floor(args->length / 2.);
        size_t left_size = left_half_upper_bound;
        size_t right_size = args->length - right_half_lower_bound;
        PyElement *Ly = (PyElement *)malloc(left_size * sizeof(PyElement));
        if (Ly == NULL)
            errExit("malloc Ly");
        PyElement *Ry = (PyElement *)malloc(right_size * sizeof(PyElement));
        if (Ry == NULL)
            errExit("malloc Ry");

        populateLy(args->Py, args->length, Ly, left_half_upper_bound);
        populateRy(args->Py, args->length, Ry, right_half_lower_bound);

        points_distance *left_result = (points_distance *)malloc(sizeof(points_distance));
        if (left_result == NULL)
            errExit("malloc left_result");
        points_distance *right_result = (points_distance *)malloc(sizeof(points_distance));
        if (right_result == NULL)
            errExit("malloc right_result");

        //new thread: works the left half
        pthread_t thread;
        thread_arg *left_args = malloc(sizeof(thread_arg));
        if (left_args == NULL)
            errExit("malloc left_args");

        left_args->Px = args->Px;
        left_args->Py = Ly;
        left_args->length = left_size;
        left_args->result = left_result;
        left_args->par_threshold = args->par_threshold;
        int tstatus = pthread_create(&thread, NULL, closest_points_multithread, left_args);
        if (tstatus != 0)
            errExit("pthread_create");

        //current thread: works the right half
        thread_arg *right_args = malloc(sizeof(thread_arg));
        if (right_args == NULL)
            errExit("malloc right_args");

        right_args->Px = args->Px + right_half_lower_bound;
        right_args->Py = Ry;
        right_args->length = right_size;
        right_args->result = right_result;
        right_args->par_threshold = args->par_threshold;
        closest_points_multithread(right_args);
        tstatus = pthread_join(thread, NULL);
        if (tstatus != 0)
            errExit("pthread_join");

        float min_left_distance = left_result->distance;
        float min_right_distance = right_result->distance;

        float min_distance_upper_bound = MIN(min_left_distance, min_right_distance);
        size_t *candidates_length = (size_t *)malloc(sizeof(size_t));
        if (candidates_length == NULL)
            errExit("malloc candidates_length");

        PyElement *candidates = get_candidates_from_different_halves(*(args->Px + left_size - 1), args->Py, args->length, candidates_length, min_distance_upper_bound);
        points_distance closest_candidates = closest_points_from_different_halves(candidates, *candidates_length);

        free(left_args);
        free(right_args);
        free(candidates_length);
        free(Ly);
        free(Ry);
        free(candidates);

        if (closest_candidates.distance < min_distance_upper_bound)
        {
            args->result->p1 = closest_candidates.p1;
            args->result->p2 = closest_candidates.p2;
            args->result->distance = closest_candidates.distance;
        }
        else if (min_left_distance < min_right_distance)
        {
            args->result->p1 = left_result->p1;
            args->result->p2 = left_result->p2;
            args->result->distance = left_result->distance;
        }
        else
        {
            args->result->p1 = right_result->p1;
            args->result->p2 = right_result->p2;
            args->result->distance = right_result->distance;
        }
        free(left_result);
        free(right_result);
    }
    else
    {
        // if the number of points is small enough, all the work is done by the current thread
        points_distance intermediate_result = closest_points(args->Px, args->Py, args->length);
        args->result->p1 = intermediate_result.p1;
        args->result->p2 = intermediate_result.p2;
        args->result->distance = intermediate_result.distance;
    }
    return NULL;
}

points_distance nlogn_solution_multithread(point P[], size_t length, int num_processes)
{
    assert(length >= 2);
    int par_threshold = ceil((float)length / num_processes);
    points_distance *ptr_result = (points_distance *)malloc(sizeof(points_distance));
    PyElement *Py = (PyElement *)malloc(length * sizeof(PyElement));
    sort_points(P, length, Py);
    thread_arg *args = malloc(sizeof(thread_arg));
    args->Px = P;
    args->Py = Py;
    args->length = length;
    args->result = ptr_result;
    args->par_threshold = par_threshold;
    closest_points_multithread(args);
    free(Py);
    free(args);
    points_distance result = *ptr_result;
    free(ptr_result);
    return result;
}

#ifdef FAB_MAIN
int main(int argc, char const *argv[])
{
    perf_test_random(nlogn_solution_multithread, 10000000, 8);    
}
#endif

//num_points=1000, num_processes=8, time=0.0010 seconds
//num_points=10000, num_processes=8, time=0.0074 seconds
//num_points=100000, num_processes=8, time=0.0993 seconds
//num_points=1000000, num_processes=8, time=1.1689 seconds
//num_points=10000000, num_processes=8, time=15.0730 seconds