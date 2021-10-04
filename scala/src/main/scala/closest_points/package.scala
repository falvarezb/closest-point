import java.nio.ByteOrder
import java.nio.channels.FileChannel
import java.nio.file.{Paths, StandardOpenOption}
import scala.collection.mutable.ListBuffer
import scala.util.Random

package object closest_points {

  import math._
  case class Point(x: Int, y: Int)
  case class PointDistance(p1: Point, p2: Point, d: Double)
  case class PyElement(p: Point, xPosition: Int)

  /**
    * Euclidean distance
    */
  def distance(point1: Point, point2: Point): Double = {
    sqrt(pow(point1.x - point2.x, 2) + pow(point1.y - point2.y, 2))
  }


  /**
    * P -> Px, Py
    */
  def sortPoints(P: Seq[Point]): (Seq[Point], Seq[PyElement]) = {
    val Px = P.sortBy(_.x)
    val Py = Px.zipWithIndex.map { case (p, idx) => PyElement(p, idx) }.sortBy(_.p.y)
    (Px, Py)
  }

  /**
    * Px, Py -> Lx, Ly
    */
  def leftHalfPoints(Px: Seq[Point], Py: Seq[PyElement]): (Seq[Point], Seq[PyElement]) = {
    val leftHalfUpperBound = ceil(Px.length / 2d).toInt
    val newPx = Px.slice(0, leftHalfUpperBound)
    val newPy = Py.filter(_.xPosition < leftHalfUpperBound)
    (newPx, newPy)
  }

  /**
    * Px, Py -> Rx, Ry
    */
  def rightHalfPoints(Px: Seq[Point], Py: Seq[PyElement]): (Seq[Point], Seq[PyElement]) = {
    val n = Px.length
    val rightHalfLowerBound = n / 2
    val newPx = Px.slice(rightHalfLowerBound, n)
    val newPy = Py.filter(_.xPosition >= rightHalfLowerBound).map(p => PyElement(p.p, p.xPosition - rightHalfLowerBound))
    (newPx, newPy)
  }

  def getCandidatesFromDifferentHalves(rightmostLeftPoint: Point, Py: Seq[PyElement], minDistanceUpperBound: Double): Seq[PyElement] = {
    Py.filter(py => abs(rightmostLeftPoint.x - py.p.x) < minDistanceUpperBound)
  }

  def globalSolution(candidates: Seq[PyElement], partialSolution: PointDistance): PointDistance = {
    var minDistance = partialSolution.d
    var closestPoints: (Point, Point) = (partialSolution.p1, partialSolution.p2)
    for (i <- candidates.indices) {
      for (j <- i + 1 until min(candidates.length, 16)) {
        val d = distance(candidates(i).p, candidates(j).p)
        if (d < minDistance) {
          minDistance = d
          closestPoints = (candidates(i).p, candidates(j).p)
        }
      }
    }
    PointDistance(closestPoints._1, closestPoints._2, minDistance)
  }

  def closestPoints(Px: Seq[Point], Py: Seq[PyElement]): PointDistance = {
    if (Px.length == 2) {
      PointDistance(Px(0), Px(1), distance(Px(0), Px(1)))
    }
    else {
      //https://stackoverflow.com/questions/44713728/why-cant-we-have-capital-letters-in-tuple-variable-declaration-in-scala
      val (lx, ly) = leftHalfPoints(Px, Py)
      val (rx, ry) = rightHalfPoints(Px, Py)

      val leftSolution = closestPoints(lx, ly)
      val rightSolution = closestPoints(rx, ry)
      val partialSolution = List(leftSolution, rightSolution).minBy(_.d)
      globalSolution(getCandidatesFromDifferentHalves(lx.last, Py, partialSolution.d), partialSolution)
    }
  }

  def readTestFile(fileName: String): Seq[Point] = {
    val channel = FileChannel.open(Paths.get(fileName), StandardOpenOption.READ)
    import java.nio.ByteBuffer
    // allocate memory to contain the whole file: downcasting!!
    val fileSize = channel.size().toInt
    val byteBuffer = ByteBuffer.allocate(fileSize)
    byteBuffer.order(ByteOrder.nativeOrder())
    channel.read(byteBuffer)
    byteBuffer.flip()
    val intBuffer = byteBuffer.asIntBuffer()
    val numPoints =  fileSize/8
    val P: ListBuffer[Point] = ListBuffer()
    for(_ <- 0.until(numPoints)) {
      P.append(Point(intBuffer.get(), intBuffer.get()))
    }
    P
  }

  def randomSample(size: Int): Seq[Point] = {
    val rand = new Random
    val sample_space_size = size * 100
    val x = Range(0,size).map(_ => rand.nextInt(sample_space_size))
    val y = Range(0,size).map(_ => rand.nextInt(sample_space_size))
    x.zip(y).map{case (x,y) => Point(x,y)}
  }

}
