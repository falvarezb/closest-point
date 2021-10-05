package closest_points

import java.nio.ByteOrder
import java.nio.channels.FileChannel
import java.nio.file.{Paths, StandardOpenOption}
import scala.collection.mutable.ListBuffer

object FileTest {

  def main(args: Array[String]): Unit = {
    val P = readTestFile(args(0))
    println(NlognSolution.solution(P))
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

}
