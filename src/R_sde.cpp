#include "sde_ssm.h"

#include "filter_smoother.h"
#include "summary.h"
#include "mcmc.h"
#include "sde_amcmc.h"


// [[Rcpp::export]]
double loglik_sde(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L, const unsigned int seed) {
  
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  unsigned int n = model.n;
  arma::cube alpha(1, n + 1, nsim_states);
  arma::mat weights(nsim_states, n + 1);
  arma::umat indices(nsim_states, n);
  return model.bsf_filter(nsim_states, L, alpha, weights, indices);
}

// [[Rcpp::export]]
Rcpp::List bsf_sde(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L, const unsigned int seed) {
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  unsigned int n = model.n;
  arma::cube alpha(1, n + 1, nsim_states);
  arma::mat weights(nsim_states, n + 1);
  arma::umat indices(nsim_states, n);
  double loglik = model.bsf_filter(nsim_states, L, alpha, weights, indices);
  
  arma::mat at(1, n + 1);
  arma::mat att(1, n + 1);
  arma::cube Pt(1, 1, n + 1);
  arma::cube Ptt(1, 1, n + 1);
  filter_summary(alpha, at, att, Pt, Ptt, weights);
  
  arma::inplace_trans(at);
  arma::inplace_trans(att);
  return Rcpp::List::create(
    Rcpp::Named("at") = at, Rcpp::Named("att") = att, 
    Rcpp::Named("Pt") = Pt, Rcpp::Named("Ptt") = Ptt, 
    Rcpp::Named("weights") = weights,
    Rcpp::Named("logLik") = loglik, Rcpp::Named("alpha") = alpha);
}

// [[Rcpp::export]]
Rcpp::List bsf_smoother_sde(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L, const unsigned int seed) {
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  unsigned int n = model.n;
  arma::cube alpha(1, n + 1, nsim_states);
  arma::mat weights(nsim_states, n + 1);
  arma::umat indices(nsim_states, n);
  double loglik = model.bsf_filter(nsim_states, L, alpha, weights, indices);
  
  arma::mat alphahat(1, n + 1);
  arma::cube Vt(1, 1, n + 1);
  
  filter_smoother(alpha, indices);
  weighted_summary(alpha, alphahat, Vt, weights.col(n));
  
  arma::inplace_trans(alphahat);
  
  return Rcpp::List::create(
    Rcpp::Named("alphahat") = alphahat, Rcpp::Named("Vt") = Vt, 
    Rcpp::Named("weights") = weights,
    Rcpp::Named("logLik") = loglik, Rcpp::Named("alpha") = alpha);
}

// [[Rcpp::export]]
Rcpp::List sde_pm_mcmc(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L, 
  const unsigned int seed, const unsigned int n_iter, 
  const unsigned int n_burnin, const unsigned int n_thin,
  const double gamma, const double target_acceptance, const arma::mat S,
  const bool end_ram, const unsigned int type) {
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  mcmc mcmc_run(n_iter, n_burnin, 
    n_thin, model.n, 1, target_acceptance, gamma, S, type);
  
  mcmc_run.pm_mcmc_bsf_sde(model, end_ram, nsim_states, L);
  
  switch (type) { 
  case 1: {
    return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 2: {
    return Rcpp::List::create(
      Rcpp::Named("alphahat") = mcmc_run.alphahat.t(), Rcpp::Named("Vt") = mcmc_run.Vt,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 3: {
    return Rcpp::List::create(
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  }
  
  return Rcpp::List::create(Rcpp::Named("error") = "error");
}

// [[Rcpp::export]]
Rcpp::List sde_da_mcmc(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L_c, const unsigned int L_f, const unsigned int seed, 
  const unsigned int n_iter, 
  const unsigned int n_burnin, const unsigned int n_thin,
  const double gamma, const double target_acceptance, const arma::mat S,
  const bool end_ram, const unsigned int type) {
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  mcmc mcmc_run(n_iter, n_burnin, 
    n_thin, model.n, 1, target_acceptance, gamma, S, type);
  
  mcmc_run.da_mcmc_bsf_sde(model, end_ram, nsim_states, L_c, L_f);
  
  switch (type) { 
  case 1: {
    return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 2: {
    return Rcpp::List::create(
      Rcpp::Named("alphahat") = mcmc_run.alphahat.t(), Rcpp::Named("Vt") = mcmc_run.Vt,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 3: {
    return Rcpp::List::create(
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  }
  return Rcpp::List::create(Rcpp::Named("error") = "error");
}

// [[Rcpp::export]]
Rcpp::List sde_is_mcmc(const arma::vec& y, const double x0, 
  const bool positive, SEXP drift_pntr, SEXP diffusion_pntr, 
  SEXP ddiffusion_pntr, SEXP log_prior_pdf_pntr, SEXP log_obs_density_pntr,
  const arma::vec& theta, const unsigned int nsim_states, 
  const unsigned int L_c, const unsigned int L_f, const unsigned int seed, 
  const unsigned int n_iter, 
  const unsigned int n_burnin, const unsigned int n_thin,
  const double gamma, const double target_acceptance, const arma::mat S,
  const bool end_ram, const unsigned int is_type, const unsigned int n_threads,
  const unsigned int type) {
  
  Rcpp::XPtr<funcPtr> xpfun_drift(drift_pntr);
  Rcpp::XPtr<funcPtr> xpfun_diffusion(diffusion_pntr);
  Rcpp::XPtr<funcPtr> xpfun_ddiffusion(ddiffusion_pntr);
  Rcpp::XPtr<prior_funcPtr> xpfun_prior(log_prior_pdf_pntr);
  Rcpp::XPtr<obs_funcPtr> xpfun_obs(log_obs_density_pntr);
  
  sde_ssm model(y, theta, x0, positive, seed, *xpfun_drift,
    *xpfun_diffusion, *xpfun_ddiffusion, *xpfun_prior, *xpfun_obs);
  
  sde_amcmc mcmc_run(n_iter, n_burnin, n_thin, model.n, 
    target_acceptance, gamma, S, type);
  
  mcmc_run.approx_mcmc(model, end_ram, nsim_states, L_c); 
  
  if(is_type == 3) {
    mcmc_run.expand();
  }
  
  mcmc_run.is_correction_bsf(model, nsim_states, L_c, L_f, is_type, n_threads);
  
  switch (type) { 
  case 1: {
    return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("weights") = mcmc_run.weight_storage,
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 2: {
    return Rcpp::List::create(
      Rcpp::Named("alphahat") = mcmc_run.alphahat.t(), Rcpp::Named("Vt") = mcmc_run.Vt,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("weights") = mcmc_run.weight_storage,
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  case 3: {
    return Rcpp::List::create(
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("weights") = mcmc_run.weight_storage,
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } break;
  }
  
  return Rcpp::List::create(Rcpp::Named("error") = "error");
}