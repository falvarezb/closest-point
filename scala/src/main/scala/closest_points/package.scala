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

    // this bound turns into linear the otherwise quadratic brute-force algorithm
    val POINTS_AHEAD_TO_EXAMINE = 15
    def solution(P: Seq[Point]): PointDistance = {
      if(P.length == 2) {
        PointDistance(P(0), P(1), distance(P(0), P(1)))
      }
      else {
        val tailSolution = solution(P.tail)
        val (p, d) = P.tail.take(POINTS_AHEAD_TO_EXAMINE).foldLeft((P.tail.head, Double.MaxValue)) { case (selectedPointDistance, nextPoint) =>
          val d = distance(P.head, nextPoint)
          if(d < selectedPointDistance._2) (nextPoint, d) else selectedPointDistance
        }
        if(d < tailSolution.d) {
          PointDistance(P.head, p, d)
        } else
          tailSolution
      }
    }

    if(candidates.length < 2)
      partialSolution
    else
      List(partialSolution, solution(candidates.map(_.p))).minBy(_.d)
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
}
