#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include "closest_point.h"

struct testcase
{
    size_t length;
    point *P;
    points_distance expected;
    int num_processes;
};

struct testcase base_case()
{
    int num_points = 2;
    point p1 = {1, 0};
    point p2 = {1, 1};
    point *P = malloc(sizeof(point) * num_points);
    P[0] = p1;
    P[1] = p2;
    return (struct testcase){num_points, P, {p1, p2, 1.0}, 4};
}

struct testcase left_half_solution()
{
    int num_points = 7;
    point *P = malloc(sizeof(point) * num_points);
    P[0] = (point){3, 9};
    P[1] = (point){1, 5};
    P[2] = (point){0, 1};
    P[3] = (point){5, 3};
    P[4] = (point){8, 6};
    P[5] = (point){20, 20};
    P[6] = (point){40, 40};
    return (struct testcase){num_points, P, {{0, 1}, {1, 5}, 4.12}, 4};
}

struct testcase right_half_solution()
{
    int num_points = 7;
    point *P = malloc(sizeof(point) * num_points);
    P[0] = (point){3, 9};
    P[1] = (point){1, 5};
    P[2] = (point){0, 1};
    P[3] = (point){5, 3};
    P[4] = (point){8, 6};
    P[5] = (point){20, 20};
    P[6] = (point){20, 21};
    return (struct testcase){num_points, P, {{20, 20}, {20, 21}, 1}, 4};
}

struct testcase inter_half_solution()
{
    int num_points = 7;
    point *P = malloc(sizeof(point) * num_points);
    P[0] = (point){2, -100};
    P[1] = (point){0, 0};
    P[2] = (point){9, 100};
    P[3] = (point){10, 0};
    P[4] = (point){11, 100};
    P[5] = (point){20, -100};
    P[6] = (point){20, 0};    
    return (struct testcase){num_points, P, {{9, 100}, {11, 100}, 2}, 4};
}

struct testcase repeat_points()
{ // repeat points
    int num_points = 4;
    point *P = malloc(sizeof(point) * num_points);
    P[0] = (point){3, 9};
    P[1] = (point){1, 5};
    P[2] = (point){10, 5};
    P[3] = (point){3, 9};
    return (struct testcase){num_points, P, {{3, 9}, {3, 9}, 0}, 4};
}

struct testcase randon_number_points()
{ //by construction, we know the expected result
    int num_points = 10;
    point *P = malloc(sizeof(point) * num_points);
    for (size_t i = 0; i < num_points - 1; i++)
    {
        P[i] = (point){i * 5, i * 5};
    }
    P[num_points - 1] = (point){10, 11};

    return (struct testcase){num_points, P, {{10, 10}, {10, 11}, 1}, 4};
}

void assert_pyelem_equal(PyElement p1, PyElement p2)
{
    assert_int_equal(p1.p.x, p2.p.x);
    assert_int_equal(p1.p.y, p2.p.y);
    assert_int_equal(p1.xposition, p2.xposition);
}

void assert_point_equal(point p1, point p2)
{
    assert_int_equal(p1.x, p2.x);
    assert_int_equal(p1.y, p2.y);
}

void assert_points_distance_equal(points_distance p1, points_distance p2)
{
    assert_float_equal(p1.distance, p2.distance, 0.01);
    // we cannot guarantee that the pair of points inside each 'points_distance' argument come in the same order,
    // therefore we need to check both possibilities
    assert_true((p1.p1.x == p2.p1.x && p1.p1.y == p2.p1.y && p1.p2.x == p2.p2.x && p1.p2.y == p2.p2.y) ||
                (p1.p1.x == p2.p2.x && p1.p1.y == p2.p2.y && p1.p2.x == p2.p1.x && p1.p2.y == p2.p1.y));
}

void compare_solutions(points_distance (*func1)(point P[], size_t length, int num_processes), points_distance (*func2)(point P[], size_t length, int num_processes))
{
    const int max_num_points = 100;
    srand(time(NULL));
    for (size_t i = 2; i < max_num_points; i++)
    {
        size_t length = i;
        point P1[length];
        point P2[length];
        for (size_t i = 0; i < length; i++)
        {
            // scale factor to expand coordinates space and make point repetition less likely
            int scale_factor = 1000;
            point *p = rand_point(0, max_num_points * scale_factor);
            P1[i] = *p;
            P2[i] = *p;
            free(p);
        }
        points_distance closest_points1 = func1(P1, length, 4);
        points_distance closest_points2 = func2(P2, length, 4);
        assert_points_distance_equal(closest_points1, closest_points2);
    }
}

void test_get_candidates_from_different_halves(void **state)
{
    (void)state; /* unused */

    PyElement p1 = {1, 2};
    PyElement p2 = {4, 2};
    PyElement p3 = {3, 2};
    PyElement p4 = {2, 2};
    PyElement Py[] = {p1, p2, p3, p4};
    point reference = p4.p;
    size_t length = 4;
    size_t *candidates_length = malloc(sizeof(size_t));
    float min_distance_upper_bound = 1.5;
    PyElement *result = get_candidates_from_different_halves(reference, Py, length, candidates_length, min_distance_upper_bound);

    assert_non_null(result);
    assert_int_equal(*candidates_length, 3);
    assert_pyelem_equal(*result, p1);
    assert_pyelem_equal(*(result + 1), p3);
    assert_pyelem_equal(*(result + 2), p4);
}

void test_get_candidates_from_different_halves_empty(void **state)
{
    (void)state; /* unused */

    PyElement p1 = {1, 2};
    PyElement p2 = {4, 2};
    PyElement p3 = {3, 2};
    PyElement p4 = {2, 2};
    PyElement Py[] = {p1, p2, p3, p4};
    point reference = p4.p;
    size_t length = 4;
    size_t *candidates_length = malloc(sizeof(size_t));
    float min_distance_upper_bound = .5;
    PyElement *result = get_candidates_from_different_halves(reference, Py, length, candidates_length, min_distance_upper_bound);

    assert_non_null(result);
    assert_int_equal(*candidates_length, 1);
    assert_pyelem_equal(*result, p4);
}

void test_sort_points(void **state)
{
    (void)state; /* unused */

    size_t length = 4;
    point p1 = {0, 0};
    point p2 = {3, 4};
    point p3 = {2, 5};
    point p4 = {1, 4};
    point P[] = {p1, p2, p3, p4};
    PyElement *Py = (PyElement *)malloc(length * sizeof(PyElement));

    sort_points(P, length, Py);

    assert_point_equal(*P, p1);
    assert_point_equal(*(P + 1), p4);
    assert_point_equal(*(P + 2), p3);
    assert_point_equal(*(P + 3), p2);

    assert_point_equal(Py->p, p1);
    assert_int_equal(Py->xposition, 0);
    assert_point_equal((Py + 1)->p, p4);
    assert_int_equal((Py + 1)->xposition, 1);
    assert_point_equal((Py + 2)->p, p2);
    assert_int_equal((Py + 2)->xposition, 3);
    assert_point_equal((Py + 3)->p, p3);
    assert_int_equal((Py + 3)->xposition, 2);
}

void test_quadratic_repeat(void **state)
{
    struct testcase tc = repeat_points();
    points_distance closest_points = quadratic_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_left_half_solution(void **state)
{
    struct testcase tc = left_half_solution();
    points_distance closest_points = nlogn_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_right_half_solution(void **state)
{
    struct testcase tc = right_half_solution();
    points_distance closest_points = nlogn_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_inter_half_solution(void **state)
{
    struct testcase tc = inter_half_solution();
    points_distance closest_points = nlogn_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_repeat(void **state)
{
    struct testcase tc = repeat_points();
    points_distance closest_points = nlogn_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_random_number_points(void **state)
{
    struct testcase tc = randon_number_points();
    points_distance closest_points = nlogn_solution(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_multiproc_base_case(void **state)
{
    struct testcase tc = base_case();
    points_distance closest_points = nlogn_solution_multiproc(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_multiproc(void **state)
{
    struct testcase tc = left_half_solution();
    points_distance closest_points = nlogn_solution_multiproc(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_multithread_base_case(void **state)
{
    struct testcase tc = base_case();
    points_distance closest_points = nlogn_solution_multithread(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_multithread(void **state)
{
    struct testcase tc = left_half_solution();
    points_distance closest_points = nlogn_solution_multithread(tc.P, tc.length, tc.num_processes);
    assert_points_distance_equal(closest_points, tc.expected);
}

void test_nlogn_vs_quadratic(void **state)
{
    compare_solutions(nlogn_solution, quadratic_solution);
}

void test_nlogn_vs_multiproc(void **state)
{
    compare_solutions(nlogn_solution, nlogn_solution_multiproc);
}

void test_multiproc_vs_multithread(void **state)
{
    compare_solutions(nlogn_solution_multithread, nlogn_solution_multiproc);
}

int main(int argc, char const *argv[])
{

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_multiproc_vs_multithread),
        cmocka_unit_test(test_nlogn_multithread),
        cmocka_unit_test(test_nlogn_multithread_base_case),
        cmocka_unit_test(test_nlogn_vs_multiproc),
        cmocka_unit_test(test_nlogn_multiproc),
        cmocka_unit_test(test_nlogn_multiproc_base_case),
        cmocka_unit_test(test_sort_points),
        cmocka_unit_test(test_nlogn_vs_quadratic),
        cmocka_unit_test(test_get_candidates_from_different_halves),
        cmocka_unit_test(test_get_candidates_from_different_halves_empty),
        cmocka_unit_test(test_nlogn_left_half_solution),
        cmocka_unit_test(test_nlogn_inter_half_solution),
        cmocka_unit_test(test_nlogn_right_half_solution),
        cmocka_unit_test(test_nlogn_repeat),
        cmocka_unit_test(test_nlogn_random_number_points),
        cmocka_unit_test(test_quadratic_repeat)
        };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
