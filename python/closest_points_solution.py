# Algorithm Design by Jon Kleinberg, Eva Tardos

"""
Given n points in the plane, find the pair that is closest together.
"""

import math
from multiprocessing import Process, Value


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __eq__(self, o: object) -> bool:
        return self.x == o.x and self.y == o.y

    def __hash__(self) -> int:
        return self.x * 31 + self.y

    def __repr__(self) -> str:
        return f"({self.x}, {self.y})"


class PointDistance:
    def __init__(self, p1, p2, d):
        self.p1 = p1
        self.p2 = p2
        self.d = d

    def __eq__(self, o: object) -> bool:
        return self.p1 == o.p1 and self.p2 == o.p2 and self.d == o.d

    def __repr__(self) -> str:
        return f"PointDistance({self.p1}, {self.p2}, {self.d})"


class PyElement:
    """
    Given a list of points P:
    - Px is the list of points sorted by coordinate x
    - Py is the list of points sorted by coordinate y

    This class represents elemenets of Py:
    - point: point (x,y) in the plane
    - x_position: position of (x,y) in Px

    We use 
    - Px to split in two halves the set of elements in each recursion
    - Py to find the solution when merging the results of the two halves in each recursion
    """

    def __init__(self, point, x_position):
        self.point = point
        self.x_position = x_position

    def __repr__(self) -> str:
        return f"[{self.point}, {self.x_position}]"

    def __eq__(self, o: object) -> bool:
        return self.point == o.point and self.x_position == o.x_position


def distance(p1: Point, p2: Point) -> float:
    """
    each tuple represents a point (x,y) in the plane
    """
    return math.sqrt((p1.x - p2.x)**2 + (p1.y - p2.y)**2)


def quadratic_solution(P) -> PointDistance:
    """
    brute force algorithm, O(n^2)

    points: list of Points
    """
    min_distance = math.inf
    for i in range(len(P)-1):
        for j in range(i+1, len(P)):
            d = distance(P[i], P[j])
            if d < min_distance:
                min_distance = d
                min_pair = (P[i], P[j])
    return PointDistance(min_pair[0], min_pair[1], min_distance)


def sort_points(P):
    """
    P -> Px, Py
    """
    Px = sorted(P, key=lambda p: p.x)
    Py = sorted([PyElement(p, i) for i, p in enumerate(Px)], key=lambda py: py.point.y)
    return Px, Py


def left_half_points(Px, Py):
    """
    returns the values Px and Py corresponding to the left half of the points
    """
    n = len(Px)
    left_half_upper_bound = math.ceil(n/2)

    newPx = Px[:left_half_upper_bound]
    newPy = [pentry for pentry in Py if pentry.x_position < left_half_upper_bound]
    return newPx, newPy


def right_half_points(Px, Py):
    """
    returns the values Px and Py corresponding to the right half of the points
    """
    n = len(Px)
    right_half_lower_bound = math.floor(n/2)

    newPx = Px[right_half_lower_bound:]
    newPy = [PyElement(pentry.point, pentry.x_position-right_half_lower_bound) for pentry in Py if pentry.x_position >= right_half_lower_bound]
    return newPx, newPy


def get_candidates_from_different_halves(rightmost_left_point, Py, min_distance_upper_bound):
    """
    Once the closest points in each half have been determined, we need to consider if the closest
    points overall belong to different halves.

    The potential candidates must lie within 'min_distance_upper_bound' of
    the middle point separating the left and right halves
    """

    return [p for p in Py if abs(p.point.x-rightmost_left_point.x) < min_distance_upper_bound]


def closest_points_from_different_halves(candidates, partial_solution):
    """
    Obtain the closest points among the previously selected candidates
    Returns the closest points and their distance to each other
    """
    min_distance = partial_solution.d
    closest_candidates = (partial_solution.p1, partial_solution.p2)
    for i in range(len(candidates)-1):
        for j in range(i+1, min(len(candidates), i+16)):
            d = distance(candidates[i].point, candidates[j].point)
            if d < min_distance:
                min_distance = d
                closest_candidates = (candidates[i].point, candidates[j].point)
    return PointDistance(closest_candidates[0], closest_candidates[1], min_distance)


def closest_points(Px, Py) -> PointDistance:
    """
    Px: list of points sorted by coordinate x
    Py: list of points sorted by coordinate y

    Recursive function, each iteration halves the input, therefore this recursive function is O(log n)
    """
    if len(Px) == 2:
        return PointDistance(Px[0], Px[1], distance(Px[0], Px[1]))

    Lx, Ly = left_half_points(Px, Py)
    Rx, Ry = right_half_points(Px, Py)

    left_solution = closest_points(Lx, Ly)
    right_solution = closest_points(Rx, Ry)
    partial_solution = min(left_solution, right_solution, key=lambda pointDistance: pointDistance.d)

    candidates = get_candidates_from_different_halves(Lx[-1], Py, partial_solution.d)
    return closest_points_from_different_halves(candidates, partial_solution)


def nlogn_solution(points):
    """
    divide and conquer algorithm, O(n log n)

    points: list of tuples, each tuple representing a point (x,y) in the plane
    """

    return closest_points(*sort_points(points))


def copy_solution_to_shared_memory(pd, shmem):
    shmem.p1.x.value = pd.p1.x
    shmem.p1.y.value = pd.p1.y
    shmem.p2.x.value = pd.p2.x
    shmem.p2.y.value = pd.p2.y
    shmem.d.value = pd.d


def closest_points_par(Px, Py, shmem, par_threshold):
    """
    Parallel version of 'closest_points'

    Calculate the number of points recursively by splitting the work and starting a new process on each recursion
    Once the level of parallelism is reached, no more processes are created and the remaining work is delegated to the
    sequential version of this function

    Each child proccess calculates the solution corresponding to its input and shares that information with its parent 
    through shared memory map

    shmem (shared memory) is a tuple of 2 points, where each point is a tuple of 2 Value objects corresponding
    to coordinates x and y
    There is no need to synchronize the access to shared memory as only the child writes to it and the parent waits for
    the child to finish before reading
    """
    if len(Px) == 2:
        copy_solution_to_shared_memory(PointDistance(Px[0], Px[1], distance(Px[0], Px[1])), shmem)

    elif len(Px) > par_threshold:
        # shared memory values to share data with child processes
        left_solution = PointDistance(Point(Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Point(
            Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Value('d', math.inf, lock=False))
        right_solution = PointDistance(Point(Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Point(
            Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Value('d', math.inf, lock=False))

        # closest points in each half
        Lx, Ly = left_half_points(Px, Py)
        Rx, Ry = right_half_points(Px, Py)

        pleft = Process(target=closest_points_par, args=(Lx, Ly, left_solution, par_threshold))
        pleft.start()
        closest_points_par(Rx, Ry, right_solution, par_threshold)
        pleft.join()
        partial_solution = min(left_solution, right_solution, key=lambda pointDistance: pointDistance.d.value)

        candidates = get_candidates_from_different_halves(Lx[-1], Py, partial_solution.d.value)
        global_solution = closest_points_from_different_halves(candidates, PointDistance(
            Point(partial_solution.p1.x.value, partial_solution.p1.y.value), Point(partial_solution.p2.x.value, partial_solution.p2.y.value), partial_solution.d.value))
        copy_solution_to_shared_memory(global_solution, shmem)
    else:
        # DEFAULTING TO SEQUENTIAL ALGORITHM
        copy_solution_to_shared_memory(closest_points(Px, Py), shmem)


def nlogn_solution_par(points, num_processes):
    """
    Parallel version of 'nlogn_solution' where num_processes is the number of processes to spawn

    num_processes MUST be a power of 2
    """
    # shared memory values: not really needed as no new process is spawned here. However, needed to respect the signature of 'closest_points_par'
    solution = PointDistance(Point(Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Point(
        Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Value('d', math.inf, lock=False))
    par_threshold = len(points)//num_processes
    closest_points_par(*sort_points(points), solution, par_threshold)
    return PointDistance(Point(solution.p1.x.value, solution.p1.y.value), Point(solution.p2.x.value, solution.p2.y.value), solution.d.value)


def read_test_file(file_name):
    from array import array
    import struct
    data = array('i')
    with open(file_name, 'rb') as f:
        data.frombytes(f.read())        
        return [Point(x,y) for x,y in list(struct.iter_unpack('ii', data))]


if __name__ == "__main__":

    from random import sample
    import timeit
    import sys

    def random_sample_test(size):
        x = sample(range(size*100), size)
        y = sample(range(size*100), size)
        return [Point(x,y) for x,y in list(zip(x, y))]

    def file_test(file_name):        
        return read_test_file(file_name)

    def main():
        # P = file_test(sys.argv[1])
        P = random_sample_test(1000000)
        print(f"nlogn_solution {timeit.timeit(lambda: nlogn_solution(P), number=1)}")
        print(f"nlogn_solution_par 4 {timeit.timeit(lambda: nlogn_solution_par(P, 4), number=1)}")
        print(f"nlogn_solution_par 8 {timeit.timeit(lambda: nlogn_solution_par(P, 8), number=1)}")

    main()
