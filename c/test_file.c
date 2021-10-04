#include "closest_point.h"

/**
 * Creates binary file with list of 'num_points' points generated randomly
 * The file can be read with the function 'perf_test_file' in the module util.c
 */
void generate_test_file(const char *filename, size_t num_points)
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

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
        errExit("fopen");

    fwrite(P, sizeof(point), num_points, file);
    fclose(file);
}

int main(int argc, char const *argv[])
{    
    int num_points = atoi(argv[2]);
    generate_test_file(argv[1], num_points);
    return 0;
}
