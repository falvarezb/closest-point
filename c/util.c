#include "closest_point.h"

/**
 * Euclidean distance
 */
float distance(point p1, point p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

/**
 * Algorithm used by sort function to compare points
 */
int compar_point(const void *p1, const void *p2)
{
    const point *point1 = (const point *)p1;
    const point *point2 = (const point *)p2;
    return (point1->x > point2->x) - (point1->x < point2->x);
}

/**
 * Algorithm used by sort function to compare PyElem
 */
int compar_PyElem(const void *p1, const void *p2)
{
    const PyElement *point1 = (const PyElement *)p1;
    const PyElement *point2 = (const PyElement *)p2;
    return (point1->p.y > point2->p.y) - (point1->p.y < point2->p.y);
}

/**
 * given P, returns Px, Py
 */
void sort_points(point P[], size_t length, PyElement Py[])
{
    //P is sorted in place and becomes Px
    qsort(P, length, sizeof(point), compar_point);

    //building Py
    for (size_t i = 0; i < length; i++)
    {
        PyElement py = {*(P + i), i};
        *(Py + i) = py;
    }
    qsort(Py, length, sizeof(PyElement), compar_PyElem);
}

/**
 * Special cases:
 * -> returns 0 elements when there are repeat points
 * -> returns 1 element when the only candidate is the point used as reference
 * 
 */
PyElement *get_candidates_from_different_halves(point reference, PyElement Py[], size_t length, size_t *candidates_length, float min_distance_upper_bound)
{
    PyElement *candidates = (PyElement *)malloc(length * sizeof(PyElement));
    if (candidates == NULL)
        errExit("malloc candidates");

    size_t j = 0;
    for (size_t i = 0; i < length; i++)
        if (abs((Py + i)->p.x - reference.x) < min_distance_upper_bound)
            *(candidates + j++) = *(Py + i);

    *candidates_length = j;
    return candidates;
}

points_distance closest_points_from_different_halves(PyElement P[], size_t length)
{
    points_distance closest_points;

    // CAUTION: guard to avoid size_t overflow
    // --> length = 0 when there are repeat points
    // --> length = 1 when the only candidate is the point
    //      used as reference to determine the candidates
    if (length < 2)
    {
        closest_points.distance = INFINITY;
        return closest_points;
    }

    float min_distance = INFINITY;
    float d;

    // CAUTION: size_t is unsigned => if length=0, length-1 is positive
    for (size_t i = 0; i < length - 1; i++)
    {
        for (size_t j = i + 1; j < MIN(length, i + 16); j++)
        {
            d = distance(P[i].p, P[j].p);
            if (d < min_distance)
            {
                min_distance = d;
                closest_points.p1 = P[i].p;
                closest_points.p2 = P[j].p;
                closest_points.distance = d;
            }
        }
    }

    return closest_points;
}

/**
 * Determine the elements of Py that correspond to Ly
 */
void populateLy(PyElement *Py, size_t Pylength, PyElement *Ly, size_t left_half_upper_bound)
{
    size_t j = 0;
    for (size_t i = 0; i < Pylength; i++)
        if ((Py + i)->xposition < left_half_upper_bound)
            *(Ly + j++) = *(Py + i);
}

/**
 * Determine the elements of Py that correspond to Ry
 */
void populateRy(PyElement *Py, size_t Pylength, PyElement *Ry, size_t right_half_lower_bound)
{
    size_t j = 0;
    for (size_t i = 0; i < Pylength; i++)
        if ((Py + i)->xposition >= right_half_lower_bound)
        {
            PyElement py = {(Py + i)->p, (Py + i)->xposition - right_half_lower_bound};
            *(Ry + j++) = py;
        }
}

/**
 * Implementation of the algorithm to find the closest points in time nlogn
 * 
 * Note: this function is defined in this module instead of nlogn_sol because is also shared 
 * by the parallel versions of the nlogn solution
 */
points_distance closest_points(point Px[], PyElement Py[], size_t length)
{
    if (length == 2)
    {
        points_distance result;
        result.p1 = Px[0];
        result.p2 = Px[1];
        result.distance = distance(result.p1, result.p2);
        return result;
    }

    // SPLIT IN HALVES:
    // even number of points => left and right halves are disjoint
    // odd number of points => left and right halves intersect in the middle point
    // this is just an implementation detail that is irrelevant for the algorithm
    size_t left_half_upper_bound = ceil(length / 2.);
    size_t right_half_lower_bound = floor(length / 2.);
    size_t left_size = left_half_upper_bound;
    size_t right_size = length - right_half_lower_bound;

    // sorting halves by coordinate y
    PyElement *Ly = (PyElement *)malloc(left_size * sizeof(PyElement));
    if (Ly == NULL)
        errExit("malloc Ly");
    PyElement *Ry = (PyElement *)malloc(right_size * sizeof(PyElement));
    if (Ry == NULL)
        errExit("malloc Ry");

    populateLy(Py, length, Ly, left_half_upper_bound);
    populateRy(Py, length, Ry, right_half_lower_bound);

    //closest points in the left half
    points_distance left_closest_points = closest_points(Px, Ly, left_size);
    float min_left_distance = left_closest_points.distance;

    //closest points in the right half
    points_distance right_closest_points = closest_points(Px + right_half_lower_bound, Ry, right_size);
    float min_right_distance = right_closest_points.distance;

    //closest points from different halves
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
        return closest_candidates;
    else if (min_left_distance < min_right_distance)
        return left_closest_points;
    else
        return right_closest_points;
}

/**
 * Return point with random coordinates whose values vary in the range [min_value, max_value]
 */
point *rand_point(int min_value, int max_value)
{
    point *p = (point *)malloc(sizeof(point));
    if (p == NULL)
        errExit("malloc point");

    p->x = rand() % (max_value - min_value + 1) + min_value;
    p->y = rand() % (max_value - min_value + 1) + min_value;
    return p;
}

void print_points_distance(points_distance p)
{
    pid_t pid = getpid();
    printf("pid=%d, x1=%d, y1=%d\n", pid, p.p1.x, p.p1.y);
    printf("pid=%d, x2=%d, y2=%d\n", pid, p.p2.x, p.p2.y);
    printf("pid=%d, d=%.2f\n", pid, p.distance);
}

/**
 * Function to run perf tests
 * It prints out the time taken to run the function "func"
 * The elements of the array P are generated randomly
 */
void perf_test_random(points_distance (*func)(point P[], size_t length, int num_processes), size_t num_points, int num_processes)
{
    srand(time(NULL));
    point *P = malloc(sizeof(point) * num_points);
    for (size_t i = 0; i < num_points; i++)
    {
        // scale factor to expand coordinates space and make point repetition less likely
        int scale_factor = 100;
        point *p = rand_point(0, num_points * scale_factor);
        P[i] = *p;
        free(p);
    }

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);
    points_distance closest_points = func(P, num_points, num_processes);

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("num_points=%ld, num_processes=%d, time=%.4f seconds\n", num_points, num_processes, elapsed);
    print_points_distance(closest_points);
    free(P);
}

/**
 * Similar to perf_test_random but elements of the array are read from a binary file generated with generate_test_file.c
 */
void perf_test_file(points_distance (*func)(point P[], size_t length, int num_processes), const char *filename, int num_processes)
{
    //point *P = malloc(sizeof(point) * num_points);

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        errExit("fopen");

    // obtain file size:
    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);
    size_t num_points = fsize/8;

    // allocate memory to contain the whole file:
    point *P = malloc(sizeof(point) * num_points);
    if (P == NULL)
        errExit("malloc P");

    // copy the file into the buffer:
    size_t result = fread(P, sizeof(point), num_points, file);
    if (result != (fsize/8))
        errExit("fread");
    fclose(file);
    

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);
    points_distance closest_points = func(P, num_points, num_processes);

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("num_points=%ld, num_processes=%d, time=%.4f seconds\n", num_points, num_processes, elapsed);
    print_points_distance(closest_points);
    free(P);   
}
