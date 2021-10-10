import math

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
    return math.sqrt((p1.x - p2.x)**2 + (p1.y - p2.y)**2)


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

    Recursive function, each iteration halves the input
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
