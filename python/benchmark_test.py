import sys
from random import sample
import timeit
from util import Point
from quadratic_sol import quadratic_solution
from nlogn_sol import nlogn_solution
from multiproc_sol import nlogn_solution_multiproc
from file_test import read_test_file


def random_sample_test(size):
    x = sample(range(size*100), size)
    y = sample(range(size*100), size)
    return [Point(x, y) for x, y in list(zip(x, y))]


if __name__ == "__main__":

    def main():
        test_mode = sys.argv[1]
        if test_mode == 'random':
            P = random_sample_test(sys.argv[2])
        else:
            P = read_test_file(sys.argv[1])

        print(f"nlogn_solution {timeit.timeit(lambda: nlogn_solution(P), number=1)}")
        print(f"nlogn_solution_par 4 {timeit.timeit(lambda: nlogn_solution_multiproc(P, 4), number=1)}")
        print(f"nlogn_solution_par 8 {timeit.timeit(lambda: nlogn_solution_multiproc(P, 8), number=1)}")

    main()
