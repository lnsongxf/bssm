context("Test models")

test_that("bad argument values for bsm throws an error",{
  expect_error(bsm("character vector"))
  expect_error(bsm(matrix(0, 2, 2)))
  expect_error(bsm(1))
  expect_error(bsm(c(1, Inf)))
  expect_error(bsm(1:10, sd_y = "character"))
  expect_error(bsm(1:10, sd_y = Inf))
  expect_error(bsm(1:10, no_argument = 5))
  expect_error(bsm(1:10, xreg = matrix(NA)))
  expect_error(bsm(1:10, xreg = matrix(1:20), beta = uniform(0, 0, 1)))
  expect_error(bsm(1:10, xreg = 1:10, beta = NA))
  expect_error(bsm(1:10, 1, 1, 1, a1 = 1))
  expect_error(bsm(1:10, 1, 1, 1, 1))
})

test_that("proper arguments for bsm don't throw an error",{
  expect_error(bsm(1:10, 1, 1), NA)
  expect_error(bsm(1:10, uniform(0, 0, 1), 1), NA)
  expect_error(bsm(1:10, 1, 1, uniform(0, 0, 1)), NA)
  expect_error(bsm(1:10, 1, 1, 1, 1, period = 3), NA)
  expect_error(bsm(1:10, 1, 1, 1, 1, period = 3, xreg = matrix(1:10, 10), beta = normal(0,0,10)), NA)
})


test_that("bad argument values for ng_bsm throws an error",{
  expect_error(ng_bsm("character vector", distribution = "poisson"))
  expect_error(ng_bsm(matrix(0, 2, 2), distribution = "poisson"))
  expect_error(ng_bsm(1, distribution = "poisson"))
  expect_error(ng_bsm(c(1, Inf), distribution = "poisson"))
  expect_error(ng_bsm(1:10, sd_level = "character", distribution = "poisson"))
  expect_error(ng_bsm(1:10, sd_y = Inf, distribution = "poisson"))
  expect_error(ng_bsm(1:10, no_argument = 5, distribution = "poisson"))
  expect_error(ng_bsm(1:10, xreg = matrix(1:20), beta = uniform(0, 0, 1), distribution = "poisson"))
  expect_error(ng_bsm(1:10, xreg = 1:10, beta = NA, distribution = "poisson"))
  expect_error(ng_bsm(1:10, 1, 1, a1 = 1, distribution = "poisson"))
  expect_error(ng_bsm(1:10, 1, 1, 1, 1, distribution = "poisson"))
})

test_that("proper arguments for ng_bsm don't throw an error",{
  expect_error(ng_bsm(1:10, 1, 1, distribution = "poisson"), NA)
  expect_error(ng_bsm(1:10, uniform(0, 0, 1), 1, distribution = "poisson"), NA)
  expect_error(ng_bsm(1:10, 1, uniform(0, 0, 1), distribution = "poisson"), NA)
  expect_error(ng_bsm(1:10, 1, 1, 1, period = 3, distribution = "poisson"), NA)
  expect_error(ng_bsm(1:10, 1, 1, 1, period = 3, xreg = matrix(1:10, 10), 
    beta = normal(0,0,10), distribution = "poisson"), NA)
})

test_that("bad argument values for svm throws an error",{
  expect_error(svm("character vector"))
  expect_error(svm(matrix(0, 2, 2)))
  expect_error(svm(1))
  expect_error(svm(c(1, Inf)))
  expect_error(svm(1:10, sd_level = "character"))
  expect_error(svm(1:10, rho = Inf))
  expect_error(svm(1:10, no_argument = 5))
  expect_error(svm(1:10, xreg = matrix(1:20), beta = uniform(0, 0, 1)))
  expect_error(svm(1:10, xreg = 1:10, beta = NA))
  expect_error(svm(1:10, 1, 1, a1 = 1))
})

test_that("proper arguments for svm don't throw an error",{
  expect_error(svm(1:10, uniform(0.9,-0.9, 0.99), halfnormal(1,2), halfnormal(1,2)), NA)
})


