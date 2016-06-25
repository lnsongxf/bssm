#include "bsm.h"

// [[Rcpp::export]]
double bsm_loglik(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, bool slope, bool seasonal,
  arma::uvec fixed, arma::mat& xreg, arma::vec& beta) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, 1);

  return model.log_likelihood();
}

// [[Rcpp::export]]
List bsm_filter(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, bool slope, bool seasonal,
  arma::uvec fixed, arma::mat& xreg, arma::vec& beta) {


  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, 1);

  arma::mat at(a1.n_elem, y.n_elem + 1);
  arma::mat att(a1.n_elem, y.n_elem);
  arma::cube Pt(a1.n_elem, a1.n_elem, y.n_elem + 1);
  arma::cube Ptt(a1.n_elem, a1.n_elem, y.n_elem);

  double logLik = model.filter(at, att, Pt, Ptt);

  arma::inplace_trans(at);
  arma::inplace_trans(att);

  return List::create(
    Named("at") = at,
    Named("att") = att,
    Named("Pt") = Pt,
    Named("Ptt") = Ptt,
    Named("logLik") = logLik);
}


// [[Rcpp::export]]
arma::mat bsm_fast_smoother(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, bool slope, bool seasonal,
  arma::uvec fixed, arma::mat& xreg, arma::vec& beta) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, 1);
  return model.fast_smoother().t();
}

// [[Rcpp::export]]
arma::cube bsm_sim_smoother(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, unsigned int nsim, bool slope,
  bool seasonal,arma::uvec fixed, arma::mat& xreg, arma::vec& beta, unsigned int seed) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed);
  return model.sim_smoother(nsim);
}


// [[Rcpp::export]]
List bsm_smoother(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, bool slope, bool seasonal,
  arma::uvec fixed, arma::mat& xreg, arma::vec& beta) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, 1);

  arma::mat alphahat(a1.n_elem, y.n_elem);
  arma::cube Vt(a1.n_elem, a1.n_elem, y.n_elem);

  model.smoother(alphahat, Vt);
  arma::inplace_trans(alphahat);

  return List::create(
    Named("alphahat") = alphahat,
    Named("Vt") = Vt);
}

// [[Rcpp::export]]
List bsm_mcmc_full(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1,
  arma::vec& theta_lwr, arma::vec& theta_upr, unsigned int n_iter,
  unsigned int nsim_states, unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat& S, bool slope,
  bool seasonal,arma::uvec fixed, arma::mat& xreg, arma::vec& beta,
  unsigned int seed, bool log_space) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  unsigned int npar = theta_lwr.n_elem;
  unsigned int n_samples = floor((n_iter - n_burnin) / n_thin);
  arma::mat theta_store(npar, n_samples);
  arma::cube alpha_store(model.m, model.n, nsim_states * n_samples);
  arma::vec ll_store(n_samples);

  double acceptance_rate = model.mcmc_full(theta_lwr, theta_upr, n_iter,
    nsim_states, n_burnin, n_thin, gamma, target_acceptance, S, alpha_store,
    theta_store, ll_store);

  arma::inplace_trans(theta_store);

  return List::create(Named("alpha") = alpha_store,
    Named("theta") = theta_store,
    Named("acceptance_rate") = acceptance_rate,
    Named("S") = S,  Named("logLik") = ll_store);

}

// [[Rcpp::export]]
List bsm_mcmc_param(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1,
  arma::vec& theta_lwr, arma::vec& theta_upr, unsigned int n_iter,
  unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat& S, bool slope,
  bool seasonal, arma::uvec fixed, arma::mat& xreg, arma::vec& beta,
  unsigned int seed, bool log_space, bool sample_states) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  unsigned int npar = theta_lwr.n_elem;
  unsigned int n_samples = floor((n_iter - n_burnin) / n_thin);
  arma::mat theta_store(npar, n_samples);
  arma::vec ll_store(n_samples);

  double acceptance_rate = model.mcmc_param(theta_lwr, theta_upr, n_iter,
    n_burnin, n_thin, gamma, target_acceptance, S, theta_store, ll_store);

  arma::inplace_trans(theta_store);

  return List::create(
    Named("theta") = theta_store,
    Named("acceptance_rate") = acceptance_rate,
    Named("S") = S,  Named("logLik") = ll_store);
}


// [[Rcpp::export]]
List bsm_mcmc_parallel_full(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1,
  arma::vec& theta_lwr, arma::vec& theta_upr, unsigned int n_iter,
  unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat& S, bool slope,
  bool seasonal, arma::uvec fixed, arma::mat& xreg, arma::vec& beta,
  unsigned int seed, bool log_space, unsigned int nsim_states,
  unsigned int n_threads, arma::uvec seeds) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  unsigned int npar = theta_lwr.n_elem;
  unsigned int n_samples = floor((n_iter - n_burnin) / n_thin);
  arma::mat theta_store(npar, n_samples);
  arma::vec ll_store(n_samples);

  double acceptance_rate = model.mcmc_param(theta_lwr, theta_upr, n_iter,
    n_burnin, n_thin, gamma, target_acceptance, S, theta_store, ll_store);

  arma::cube alpha = sample_states(model, theta_store, nsim_states, n_threads, seeds);

  arma::inplace_trans(theta_store);

  return List::create(Named("alpha") = alpha,
    Named("theta") = theta_store,
    Named("acceptance_rate") = acceptance_rate,
    Named("S") = S,  Named("logLik") = ll_store);
}

// [[Rcpp::export]]
List bsm_mcmc_summary(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, arma::vec& theta_lwr,
  arma::vec& theta_upr, unsigned int n_iter, unsigned int n_burnin,
  unsigned int n_thin, double gamma, double target_acceptance, arma::mat& S,
  bool slope, bool seasonal,arma::uvec fixed, arma::mat& xreg, arma::vec& beta,
  unsigned int seed, bool log_space) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  unsigned int npar = theta_lwr.n_elem;
  unsigned int n_samples = floor((n_iter - n_burnin) / n_thin);
  arma::mat theta_store(npar, n_samples);
  arma::vec ll_store(n_samples);
  arma::mat alphahat(model.m, model.n, arma::fill::zeros);
  arma::cube Vt(model.m, model.m, model.n, arma::fill::zeros);

  double acceptance_rate = model.mcmc_summary(theta_lwr, theta_upr, n_iter, n_burnin, n_thin,
    gamma, target_acceptance, S, alphahat, Vt, theta_store, ll_store);

  arma::inplace_trans(alphahat);
  arma::inplace_trans(theta_store);
  return List::create(Named("alphahat") = alphahat,
    Named("Vt") = Vt, Named("theta") = theta_store,
    Named("acceptance_rate") = acceptance_rate,
    Named("S") = S,  Named("logLik") = ll_store);
}


// [[Rcpp::export]]
arma::mat bsm_predict2(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, arma::vec& theta_lwr,
  arma::vec& theta_upr, unsigned int n_iter, unsigned int nsim_states,
  unsigned int n_burnin, unsigned int n_thin, double gamma,
  double target_acceptance, arma::mat& S, unsigned int n_ahead,
  unsigned int interval, bool slope, bool seasonal,arma::uvec fixed,
  arma::mat& xreg, arma::vec& beta, unsigned int seed, bool log_space) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  return model.predict2(theta_lwr, theta_upr, n_iter, nsim_states, n_burnin,
    n_thin, gamma, target_acceptance, S, n_ahead, interval);
}


// [[Rcpp::export]]
List bsm_predict(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1, arma::vec& theta_lwr,
  arma::vec& theta_upr, unsigned int n_iter,
  unsigned int n_burnin, unsigned int n_thin, double gamma,
  double target_acceptance, arma::mat& S, unsigned int n_ahead,
  unsigned int interval, bool slope, bool seasonal,arma::uvec fixed,
  arma::mat& xreg, arma::vec& beta, arma::vec probs, unsigned int seed, bool log_space) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, seed, log_space);

  return model.predict(theta_lwr, theta_upr, n_iter, n_burnin,
    n_thin, gamma, target_acceptance, S, n_ahead, interval, probs);
}

// [[Rcpp::export]]
arma::cube bsm_sample_states(arma::vec& y, arma::mat& Z, arma::vec& H, arma::cube& T,
  arma::cube& R, arma::vec& a1, arma::mat& P1,
  arma::mat& theta, unsigned int nsim_states, bool slope,
  bool seasonal,arma::uvec fixed, arma::mat& xreg, arma::vec& beta,
  unsigned int n_threads, arma::uvec seeds) {

  bsm model(y, Z, H, T, R, a1, P1, slope, seasonal, fixed, xreg, beta, 1);

  return sample_states(model, theta, nsim_states, n_threads, seeds);
}