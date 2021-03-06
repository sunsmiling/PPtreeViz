% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExp-add.R
\name{Lrindex}
\alias{Lrindex}
\title{Lr PPindex}
\usage{
Lrindex(origclass,origdata,proj,weight=TRUE,r=1)
}
\arguments{
\item{origclass}{class information vector}

\item{origdata}{data matrix without class information}

\item{proj}{projection matrix}

\item{weight}{weight flag in Lr index}

\item{r}{r in Lr index}
}
\description{
Lr projection pursuit index
}
\details{
Calculate Lr projection pursuit index value
}
\examples{
data(iris)
Lrindex(iris[,5],as.matrix(iris[,1:4]),r=1)

  
}
\references{
Lee, EK., Cook, D., Klinke, S., and Lumley, T.(2005) 
Projection Pursuit for exploratory supervised classification, 
Journal of Computational and Graphical statistics, 14(4):831-846.
}
\keyword{projection}
\keyword{pursuit}
