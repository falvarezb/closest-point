#include "closest_point.h"

points_distance quadratic_solution(point P[], size_t length, int num_processes)
{
    assert(length >= 2);
    float min_distance = INFINITY;
    float d;
    points_distance closest_points;

    for (size_t i = 0; i < length - 1; i++)
    {
        for (size_t j = i + 1; j < length; j++)
        {
            d = distance(P[i], P[j]);
            if (d < min_distance)
            {
                min_distance = d;
                closest_points.p1 = P[i];
                closest_points.p2 = P[j];
                closest_points.distance = d;
            }
        }
    }
    return closest_points;
}

#ifdef FAB_MAIN
int main(int argc, char const *argv[])
{    
    perf_test_random(quadratic_solution, 100000, -1);
}
#endif

//num_points=1000, num_processes=-1, time=0.0451 seconds
//num_points=10000, num_processes=-1, time=3.7744 seconds
//num_points=100000, num_processes=-1, time=378.8448 seconds
