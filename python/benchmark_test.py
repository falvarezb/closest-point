import sys
from random import sample
import timeit
from util import Point
from quadratic_sol import quadratic_solution
from nlogn_sol import nlogn_solution
from multiproc_sol import nlogn_solution_multiproc
from file_test import read_test_file


def random_sample(size):
    x = sample(range(size*100), size)
    y = sample(range(size*100), size)
    return [Point(x, y) for x, y in list(zip(x, y))]


if __name__ == "__main__":

    def main():
        if len(sys.argv) < 2:
            print("Usage: python benchmark_test.py test_mode [sample_size|filename]")
            exit(1)

        test_mode = sys.argv[1]
        if test_mode == 'random':
            if len(sys.argv) < 3:
                print("Usage: python benchmark_test.py 'random' sample_size")
                exit(1)
            P = random_sample(int(sys.argv[2]))
        elif test_mode == 'file':
            if len(sys.argv) < 3:
                print("Usage: python benchmark_test.py 'file' filename")
                exit(1)
            P = read_test_file(sys.argv[2])
        else:
            print("test mode must be one of: 'random', 'file'")
            exit(1)

        print(f"nlogn_solution {timeit.timeit(lambda: nlogn_solution(P), number=1)}")
        print(f"nlogn_solution_par 4 {timeit.timeit(lambda: nlogn_solution_multiproc(P, 4), number=1)}")
        print(f"nlogn_solution_par 8 {timeit.timeit(lambda: nlogn_solution_multiproc(P, 8), number=1)}")

    main()
