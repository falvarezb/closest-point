package closest_points

import closest_points.FileTest.readTestFile
import org.scalameter._
import scala.util.Random

object BenchmarkTest {

  val standardConfig = config(
    Key.exec.minWarmupRuns -> 2,
    Key.exec.maxWarmupRuns -> 2,
    Key.exec.benchRuns -> 2
  )

  def main(args: Array[String]): Unit = {
    val testMode = args(0)
    val P: Seq[Point] = if(testMode == "random")
      randomSample(args(1).toInt)
    else if(testMode == "file")
      readTestFile(args(1))
    else
      throw new IllegalArgumentException

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
