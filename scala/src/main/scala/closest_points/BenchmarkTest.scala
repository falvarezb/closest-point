package closest_points

import org.scalameter._

object BenchmarkTest {

  val standardConfig = config(
    Key.exec.minWarmupRuns -> 2,
    Key.exec.maxWarmupRuns -> 2,
    Key.exec.benchRuns -> 2
  )

  def main(args: Array[String]): Unit = {
    //val P = readTestFile(args(0))
    val P = randomSample(1000000)
    //println(s"QuadraticSolution ${standardConfig withWarmer new Warmer.Default measure {QuadraticSolution.solution(P)}}")
    val nlognSolution = standardConfig withWarmer new Warmer.Default measure {NlognSolution.solution(P)}
    val multithreadSolution4 = standardConfig withWarmer new Warmer.Default measure {MultithreadSolution.solution(P,4)}
    val multithreadSolution8 = standardConfig withWarmer new Warmer.Default measure {MultithreadSolution.solution(P,8)}
    println(s"NlognSolution ${nlognSolution.value} ${nlognSolution.units}")
    println(s"MultithreadSolution4 ${multithreadSolution4.value} ${multithreadSolution4.units}")
    println(s"MultithreadSolution8 ${multithreadSolution8.value} ${multithreadSolution8.units}")
  }

}
