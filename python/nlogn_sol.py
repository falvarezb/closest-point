from util import closest_points, sort_points


def nlogn_solution(points):
    """
    divide and conquer algorithm, O(n log n)

    points: list of points
    """

    return closest_points(*sort_points(points))