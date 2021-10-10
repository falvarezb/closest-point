from multiprocessing import Process, Value
from util import *

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


def nlogn_solution_multiproc(points, num_processes):
    """
    Multiprocess version of 'nlogn_solution' where num_processes is the number of processes to spawn

    num_processes MUST be a power of 2
    """
    # shared memory values: not really needed as no new process is spawned here. However, needed to respect the signature of 'closest_points_par'
    solution = PointDistance(Point(Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Point(
        Value('d', math.inf, lock=False), Value('d', math.inf, lock=False)), Value('d', math.inf, lock=False))
    par_threshold = len(points)//num_processes
    closest_points_par(*sort_points(points), solution, par_threshold)
    return PointDistance(Point(solution.p1.x.value, solution.p1.y.value), Point(solution.p2.x.value, solution.p2.y.value), solution.d.value)
