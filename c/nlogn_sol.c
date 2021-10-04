#include "closest_point.h"

points_distance nlogn_solution(point P[], size_t length, int num_processes)
{
    assert(length >= 2);
    PyElement *Py = (PyElement *)malloc(length * sizeof(PyElement));
    sort_points(P, length, Py);
    points_distance result = closest_points(P, Py, length);
    free(Py);
    return result;
}

#ifdef FAB_MAIN
int main(int argc, char const *argv[])
{    
    //perf_test_random(nlogn_solution, 10000000, -1);  
    perf_test_file(nlogn_solution, argv[1], -1);     
}
#endif

//num_points=1000, num_processes=-1, time=0.0020 seconds
//num_points=10000, num_processes=-1, time=0.0299 seconds
//num_points=100000, num_processes=-1, time=0.4597 seconds
//num_points=1000000, num_processes=-1, time=5.5418 seconds
//num_points=10000000, num_processes=-1, time=76.0372 seconds