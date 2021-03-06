#include "ugg_ssm.h"
#include "ugg_bsm.h"
#include "ung_ssm.h"
#include "ung_bsm.h"
#include "ung_svm.h"
#include "nlg_ssm.h"
#include "ung_ar1.h"
#include "ugg_ar1.h"

// [[Rcpp::export]]
Rcpp::List gaussian_predict(const Rcpp::List& model_,
  const arma::vec& probs, const arma::mat theta, const arma::mat alpha, 
  const arma::uvec& counts, const unsigned int predict_type,
  const bool intervals, const unsigned int seed, const int model_type, 
  const unsigned int nsim, const arma::uvec& Z_ind,
  const arma::uvec& H_ind, const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  switch (model_type) {
  case 1: {
  ugg_ssm model(clone(model_), seed, Z_ind, H_ind, T_ind, R_ind);
  if (intervals) {
    return model.predict_interval(probs, theta, alpha, counts, predict_type);
  } else {
    return Rcpp::List::create(model.predict_sample(theta, alpha, counts,
      predict_type, nsim));
  }
} break;
  case 2: {
    ugg_bsm model(clone(model_), seed);
    if (intervals) {
      return model.predict_interval(probs, theta, alpha, counts, predict_type);
    } else {
      return Rcpp::List::create(model.predict_sample(theta, alpha, counts, 
        predict_type, nsim));
    }
  } break;
  case 3: {
    ugg_ar1 model(clone(model_), seed);
    if (intervals) {
      return model.predict_interval(probs, theta, alpha, counts, predict_type);
    } else {
      return Rcpp::List::create(model.predict_sample(theta, alpha, counts, 
        predict_type, nsim));
    }
  } break;
  }
  return Rcpp::List::create(Rcpp::Named("error") = std::numeric_limits<double>::infinity());
}

// [[Rcpp::export]]
arma::cube nongaussian_predict(const Rcpp::List& model_,
  const arma::vec& probs, const arma::mat& theta, const arma::mat& alpha, 
  const arma::uvec& counts, const unsigned int predict_type, 
  const unsigned int seed, const unsigned int model_type, const unsigned int nsim,
  const arma::uvec& Z_ind, const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  switch (model_type) {
  case 1: {
  ung_ssm model(clone(model_), seed, Z_ind, T_ind, R_ind);
  
  return model.predict_sample(theta, alpha, counts, predict_type, nsim);
} break;
  case 2: {
    ung_bsm model(clone(model_), seed);
    return model.predict_sample(theta, alpha, counts, predict_type, nsim);
  } break;
  case 3: {
    ung_svm model(clone(model_), seed);
    return model.predict_sample(theta, alpha, counts, predict_type, nsim);
  } break;
  case 4: {
    ung_ar1 model(clone(model_), seed);
    return model.predict_sample(theta, alpha, counts, predict_type, nsim);
  } break;
  }
  return arma::cube(0,0,0);
}

// [[Rcpp::export]]
arma::cube nonlinear_predict(const arma::mat& y, SEXP Z, SEXP H, 
  SEXP T, SEXP R, SEXP Zg, SEXP Tg, SEXP a1, SEXP P1, 
  SEXP log_prior_pdf, const arma::vec& known_params, 
  const arma::mat& known_tv_params, const arma::uvec& time_varying, 
  const unsigned int n_states, const unsigned int n_etas,
  const arma::vec& probs, const arma::mat& theta, const arma::mat& alpha, 
  const arma::uvec& counts, const unsigned int predict_type, 
  const unsigned int seed, const unsigned int nsim) {
  
  
  Rcpp::XPtr<nvec_fnPtr> xpfun_Z(Z);
  Rcpp::XPtr<nmat_fnPtr> xpfun_H(H);
  Rcpp::XPtr<nvec_fnPtr> xpfun_T(T);
  Rcpp::XPtr<nmat_fnPtr> xpfun_R(R);
  Rcpp::XPtr<nmat_fnPtr> xpfun_Zg(Zg);
  Rcpp::XPtr<nmat_fnPtr> xpfun_Tg(Tg);
  Rcpp::XPtr<a1_fnPtr> xpfun_a1(a1);
  Rcpp::XPtr<P1_fnPtr> xpfun_P1(P1);
  Rcpp::XPtr<prior_fnPtr> xpfun_prior(log_prior_pdf);
  
  nlg_ssm model(y, *xpfun_Z, *xpfun_H, *xpfun_T, *xpfun_R, *xpfun_Zg, *xpfun_Tg, 
    *xpfun_a1, *xpfun_P1, theta.col(0), *xpfun_prior, known_params, known_tv_params, n_states, n_etas,
    time_varying, seed);
  
  return model.predict_sample(theta, alpha, counts, predict_type, nsim);
  
}

// [[Rcpp::export]]
Rcpp::List nonlinear_predict_ekf(const arma::mat& y, SEXP Z, SEXP H, 
  SEXP T, SEXP R, SEXP Zg, SEXP Tg, SEXP a1, SEXP P1, 
  SEXP log_prior_pdf, const arma::vec& known_params, 
  const arma::mat& known_tv_params, const arma::uvec& time_varying, 
  const unsigned int n_states, const unsigned int n_etas,
  const arma::vec& probs, const arma::mat& theta, const arma::mat& alpha_last, const arma::cube P_last, 
  const arma::uvec& counts, const unsigned int predict_type) {
  
  Rcpp::XPtr<nvec_fnPtr> xpfun_Z(Z);
  Rcpp::XPtr<nmat_fnPtr> xpfun_H(H);
  Rcpp::XPtr<nvec_fnPtr> xpfun_T(T);
  Rcpp::XPtr<nmat_fnPtr> xpfun_R(R);
  Rcpp::XPtr<nmat_fnPtr> xpfun_Zg(Zg);
  Rcpp::XPtr<nmat_fnPtr> xpfun_Tg(Tg);
  Rcpp::XPtr<a1_fnPtr> xpfun_a1(a1);
  Rcpp::XPtr<P1_fnPtr> xpfun_P1(P1);
  Rcpp::XPtr<prior_fnPtr> xpfun_prior(log_prior_pdf);
  
  nlg_ssm model(y, *xpfun_Z, *xpfun_H, *xpfun_T, *xpfun_R, *xpfun_Zg, *xpfun_Tg, 
    *xpfun_a1, *xpfun_P1, theta.col(0), *xpfun_prior, known_params, known_tv_params, n_states, n_etas,
    time_varying, 1);
  return model.predict_interval(probs, theta,
    alpha_last, P_last, counts, predict_type);
}