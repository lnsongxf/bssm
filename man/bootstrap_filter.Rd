% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/particle_filter.R
\name{bootstrap_filter}
\alias{bootstrap_filter}
\alias{bootstrap_filter.ngssm}
\alias{bootstrap_filter.nlg_ssm}
\alias{bootstrap_filter.svm}
\title{Particle Filtering}
\usage{
bootstrap_filter(object, nsim, ...)

\method{bootstrap_filter}{ngssm}(object, nsim,
  seed = sample(.Machine$integer.max, size = 1), ...)

\method{bootstrap_filter}{svm}(object, nsim,
  seed = sample(.Machine$integer.max, size = 1), ...)

\method{bootstrap_filter}{nlg_ssm}(object, nsim,
  seed = sample(.Machine$integer.max, size = 1), ...)
}
\arguments{
\item{object}{of class \code{bsm}, \code{ng_bsm} or \code{svm}.}

\item{nsim}{Number of samples.}

\item{...}{Ignored.}

\item{seed}{Seed for RNG.}
}
\value{
A list containing samples, weights from the last time point, and an
estimate of log-likelihood.
}
\description{
Function \code{bootstrap_filter} performs a bootstrap filtering with stratification
resampling. For non-Gaussian models, psi-auxiliary particle filter based on 
the Gaussian approximating model is also available.
}
