package closest_points

object NlognSolution {

  def solution(P: Seq[Point]): PointDistance = {
    (closestPoints _).tupled(sortPoints(P))
  }

}
