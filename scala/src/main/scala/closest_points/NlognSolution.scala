package closest_points

import closest_points.BenchmarkTest.standardConfig
import org.scalameter.Warmer

object NlognSolution {

  def solution(P: Seq[Point]): PointDistance = {
    (closestPoints _).tupled(sortPoints(P))
  }

  def main(args: Array[String]): Unit = {
    //val P = readTestFile(args(0))
    val P = randomSample(100000)
    println(standardConfig withWarmer new Warmer.Default measure {solution(P)})
    //println(solution(P))
  }

}
