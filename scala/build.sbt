name := "closest-points"
version := "0.1"
scalaVersion := "2.12.8"

libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.3" % "test",
  "com.storm-enroute" %% "scalameter" % "0.17",
  "org.scalacheck" %% "scalacheck" % "1.14.0" % "test",
  // logging
  "com.typesafe.scala-logging" %% "scala-logging" % "3.5.0",
  "ch.qos.logback" % "logback-classic" % "1.2.1",
  "com.lihaoyi" %% "os-lib" % "0.2.7"
)