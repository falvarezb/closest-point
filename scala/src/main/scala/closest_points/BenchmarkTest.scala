package closest_points

import org.scalameter._

import scala.util.Random

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

  def randomSample(size: Int): Seq[Point] = {
    val rand = new Random
    val sample_space_size = size * 100
    val x = Range(0,size).map(_ => rand.nextInt(sample_space_size))
    val y = Range(0,size).map(_ => rand.nextInt(sample_space_size))
    x.zip(y).map{case (x,y) => Point(x,y)}
  }
}
