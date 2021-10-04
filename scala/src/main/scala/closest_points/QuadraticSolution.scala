package closest_points

object QuadraticSolution {
  /**
    * Brute force solution
    */
  def solution(P: Seq[Point]): PointDistance = {
    if(P.length == 2) {
      PointDistance(P(0), P(1), distance(P(0), P(1)))
    }
    else {
      val tailSolution = solution(P.tail)
      val (p, d) = P.tail.foldLeft((P.tail.head, Double.MaxValue)) { case (selectedPointDistance, nextPoint) =>
        val d = distance(P.head, nextPoint)
        if(d < selectedPointDistance._2) (nextPoint, d) else selectedPointDistance
      }
      if(d < tailSolution.d) {
        PointDistance(P.head, p, d)
      } else
        tailSolution
    }
  }
}
