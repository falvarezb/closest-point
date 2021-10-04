package closest_points

import java.util.concurrent.{ForkJoinPool, ForkJoinTask, RecursiveTask}

class ClosestPointTask(Px: Seq[Point], Py: Seq[PyElement], parThreshold: Int) extends RecursiveTask[PointDistance] {
  override def compute(): PointDistance = {
    if(Px.length == 2) {
      PointDistance(Px(0), Px(1), distance(Px(0), Px(1)))
    }
    else if(Px.length > parThreshold) {
      //https://stackoverflow.com/questions/44713728/why-cant-we-have-capital-letters-in-tuple-variable-declaration-in-scala
      val (lx, ly) = leftHalfPoints(Px, Py)
      val (rx,ry) = rightHalfPoints(Px, Py)

      val leftClosestPointsTask: ForkJoinTask[PointDistance] = new ClosestPointTask(lx, ly, parThreshold).fork()
      val rightSolution: PointDistance = new ClosestPointTask(rx, ry, parThreshold).compute()
      val leftSolution: PointDistance = leftClosestPointsTask.join()
      val partialSolution = List(leftSolution, rightSolution).minBy(_.d)
      globalSolution(getCandidatesFromDifferentHalves(lx.last, Py, partialSolution.d), partialSolution)
    }
    else {
      closestPoints(Px, Py)
    }
  }
}

object MultithreadSolution {
  def solution(P: Seq[Point], numProcesses: Int): PointDistance = {
    val (px, py) = sortPoints(P)
    val parThreshold = P.length / numProcesses
    val task = new ClosestPointTask(px, py, parThreshold)
    val pool = new ForkJoinPool(numProcesses)
    pool.invoke(task)
  }
}
