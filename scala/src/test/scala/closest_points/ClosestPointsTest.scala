package closest_points

import org.scalacheck.Gen
import org.scalatest.prop.PropertyChecks
import org.scalatest.{Assertion, FunSpec}

import java.math.MathContext

class ClosestPointsTest extends FunSpec with PropertyChecks {

  def assertPointDistance(pd1: PointDistance, pd2: PointDistance): Assertion = {
    assert(
      (pd1.p1 == pd2.p1 && pd1.p2 == pd2.p2) ||
        (pd1.p1 == pd2.p2 && pd1.p2 == pd2.p1)
    )
    assert(BigDecimal(pd1.d, new MathContext(2)) == BigDecimal(pd2.d, new MathContext(2)))
  }

  //==== FUNCTIONAL TESTING ======

  describe("quadratic solution"){
    it("P=[(0, 1), (0, 3), (2, 0), (0, 0)]"){
      assert(QuadraticSolution.solution(List(Point(0,1), Point(0,3), Point(2,0), Point(0,0))) == PointDistance(Point(0,1),Point(0,0),1))
    }
  }

  def splitEvenPointsFixture = new {
    val p1 = Point(0,0)
    val p2 = Point(1,4)
    val p3 = Point(2,5)
    val p4 = Point(3,4)
    val Px = List(p1,p2,p3,p4)
    val Py = List(PyElement(p1,0),PyElement(p2,1),PyElement(p3,2),PyElement(p4,3))
  }

  def splitOddPointsFixture = new {
    val p1 = Point(0,0)
    val p2 = Point(1,4)
    val p3 = Point(2,5)
    val p4 = Point(3,4)
    val p5 = Point(4,0)
    val Px = List(p1,p2,p3,p4,p5)
    val Py = List(PyElement(p1,0),PyElement(p2,1),PyElement(p3,2),PyElement(p4,3),PyElement(p5,4))
  }

  describe("left half points") {
    val f = splitEvenPointsFixture
    import f._
    it("even number of points") {
      val (newPx, newPy) = leftHalfPoints(Px,Py)
      assert(newPx == List(p1,p2))
      assert(newPy == List(PyElement(p1,0),PyElement(p2,1)))
    }

    it("odd number of points") {
      val f = splitOddPointsFixture
      import f._
      val (newPx, newPy) = leftHalfPoints(Px,Py)
      assert(newPx == List(p1,p2,p3))
      assert(newPy == List(PyElement(p1,0),PyElement(p2,1),PyElement(p3,2)))
    }
  }

  describe("right half points") {
    val f = splitEvenPointsFixture
    import f._
    it("even number of points") {
      val (newPx, newPy) = rightHalfPoints(Px,Py)
      assert(newPx == List(p3,p4))
      assert(newPy == List(PyElement(p3,0),PyElement(p4,1)))
    }

    it("odd number of points") {
      val f = splitOddPointsFixture
      import f._
      val (newPx, newPy) = rightHalfPoints(Px,Py)
      assert(newPx == List(p3,p4,p5))
      assert(newPy == List(PyElement(p3,0),PyElement(p4,1),PyElement(p5,2)))
    }
  }

  describe("sortPoints") {
    it("[(0, 0), (3, 4), (2, 5), (1, 4)]") {
      val f = splitEvenPointsFixture
      import f._
      val P = List(p1,p4,p3,p2)
      val (newPx, newPy) = sortPoints(P)
      assert(newPx == List(p1,p2,p3,p4))
      assert(newPy == List(PyElement(p1, 0), PyElement(p2, 1), PyElement(p4, 3), PyElement(p3, 2)))
    }
  }

  describe("nlogn solution"){
    it("left half solution"){
      val P = List(Point(3,9),Point(1,5),Point(0,1),Point(5,3),Point(8,6),Point(20,20),Point(40,40))
      assertPointDistance(NlognSolution.solution(P), PointDistance(Point(0,1),Point(1,5), 4.12))
    }
    it("right half solution"){
      val P = List(Point(3,9),Point(1,5),Point(0,1),Point(5,3),Point(8,6),Point(20,20),Point(20,21))
      assertPointDistance(NlognSolution.solution(P), PointDistance(Point(20,20),Point(20,21), 1))
    }
    it("inter halves solution"){
      val P = List(Point(2,-100),Point(0,0),Point(9,100),Point(10,0),Point(11,100),Point(20,-100),Point(20,0))
      assertPointDistance(NlognSolution.solution(P), PointDistance(Point(9,100),Point(11,100), 2))
    }
    it("repeat points"){
      val P = List(Point(3,9),Point(1,5),Point(10,5),Point(3,9))
      assertPointDistance(NlognSolution.solution(P), PointDistance(Point(3,9),Point(3,9), 0))
    }
    it("P=[(0, 1), (0, 3), (2, 0), (0, 0)]"){
      assert(NlognSolution.solution(List(Point(0,1), Point(0,3), Point(2,0), Point(0,0))) == PointDistance(Point(0,0),Point(0,1),1))
    }
  }

  describe("multithread solution") {
    describe("P=[(0, 1), (0, 3), (2, 0), (0, 0)]") {
      it("single thread behaviour") {
        assert(MultithreadSolution.solution(List(Point(0, 1), Point(0, 3), Point(2, 0), Point(0, 0)), 1) == PointDistance(Point(0, 0), Point(0, 1), 1))
      }
      it("multi thread behaviour") {
        assert(MultithreadSolution.solution(List(Point(0, 1), Point(0, 3), Point(2, 0), Point(0, 0)), 4) == PointDistance(Point(0, 0), Point(0, 1), 1))
      }
    }
  }

  //==== PROPERTY-BASED TESTING ======

  /*
  Shrinking ignores restrictions of the generator, e.g. shrinking a Gen.choose(1, 10) might try to falsify the
  property with values such as 0, and -1

  With this statement, shrinking is disabled
  */
  import org.scalacheck.Shrink.shrinkAny

  def pointsGenerator(): Gen[Seq[Point]] = for {
    len <- Gen.choose(2, 100)
    x <- Gen.pick(len, 1 to 1000)
    y <- Gen.pick(len, 1 to 1000)
  } yield x.zip(y).map{case (x,y) => Point(x,y)}


  forAll(pointsGenerator()) { P =>
    assertPointDistance(NlognSolution.solution(P), QuadraticSolution.solution(P))
    assertPointDistance(NlognSolution.solution(P), MultithreadSolution.solution(P,4))
  }

}
