\name{listen}
\alias{listen}
\docType{package}
\title{
    Listen to filesystem events
}
\description{
    Listen to filesystem events
}
\usage{
listen(path, callback)
}
\arguments{
  \item{path}{A character vector giving the path to a file. Filesystem events involving this file will trigger the callback}

  \item{callback}{A function to be called when an event occurs}
}
\value{
    None
}
\examples{
\dontrun{
library(fileuv)
library(liteq)
db <- tempfile()
queue <- ensure_queue("events", db = db)
f <- function() cat("event received\n")
s <- listen(db, f)
publish(queue, title = "event", message = "hi")
}
}
