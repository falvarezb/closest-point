import sys
from util import Point
from quadratic_sol import quadratic_solution
from nlogn_sol import nlogn_solution
from multiproc_sol import nlogn_solution_multiproc


def read_test_file(file_name):
    from array import array
    import struct
    data = array('i')
    with open(file_name, 'rb') as f:
        data.frombytes(f.read())
        return [Point(x, y) for x, y in list(struct.iter_unpack('ii', data))]


if __name__ == "__main__":

    def main():
        if len(sys.argv) < 2:
            print("Usage: python file_test.py filename")
            exit(1)

        P = read_test_file(sys.argv[1])
        solution = nlogn_solution(P)
        print(solution)
        assert quadratic_solution(P).d == solution.d
        assert nlogn_solution_multiproc(P, 4) == solution

    main()
