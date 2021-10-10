import math
from util import distance, PointDistance 


def quadratic_solution(P) -> PointDistance:
    """
    brute force algorithm, O(n^2)
    """
    min_distance = math.inf
    for i in range(len(P)-1):
        for j in range(i+1, len(P)):
            d = distance(P[i], P[j])
            if d < min_distance:
                min_distance = d
                min_pair = (P[i], P[j])
    return PointDistance(min_pair[0], min_pair[1], min_distance)