#include "mcmc.h"
#include "ung_amcmc.h"
#include "ugg_bsm.h"
#include "ung_bsm.h"

// [[Rcpp::export]]
Rcpp::List gaussian_mcmc(const Rcpp::List& model_,
  arma::uvec prior_types, arma::mat prior_pars, bool sim_states,
  unsigned int n_iter, unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat S,
  unsigned int seed, bool end_ram, unsigned int n_threads, 
  int model_type, const arma::uvec& Z_ind, const arma::uvec& H_ind, 
  const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  arma::vec a1 = Rcpp::as<arma::vec>(model_["a1"]);
  unsigned int m = a1.n_elem;
  unsigned int n;
  
  if(model_type > 0) {
    arma::vec y = Rcpp::as<arma::vec>(model_["y"]);
    n = y.n_elem;
  } else {
    arma::vec y = Rcpp::as<arma::mat>(model_["y"]);
    n = y.n_rows;
  }
  
  mcmc mcmc_run(prior_types, prior_pars, n_iter, n_burnin, n_thin, n, m,
    target_acceptance, gamma, S, sim_states);
  
  switch (model_type) {
  case 1: {
    ugg_ssm model(clone(model_), seed, Z_ind, H_ind, T_ind, R_ind);
    mcmc_run.mcmc_gaussian(model, end_ram);
    if(sim_states) mcmc_run.state_posterior(model, n_threads);
  } break;
  case 2: {
    ugg_bsm model(clone(model_), seed);
    mcmc_run.mcmc_gaussian(model, end_ram);
    if(sim_states) mcmc_run.state_posterior(model, n_threads);
  } break;
  }
  
  if(sim_states) {
    return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
      Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  } else {
    return Rcpp::List::create(Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
      Rcpp::Named("counts") = mcmc_run.count_storage,
      Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
      Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
  }
}

// [[Rcpp::export]]
Rcpp::List nongaussian_pm_mcmc(const Rcpp::List& model_,
  arma::uvec prior_types, arma::mat prior_pars, unsigned int nsim_states, 
  unsigned int n_iter, unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat S,
  unsigned int seed, bool end_ram, unsigned int n_threads, bool local_approx,
  arma::vec initial_mode, unsigned int max_iter, double conv_tol,
  unsigned int simulation_method, int model_type, const arma::uvec& Z_ind,
  const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  arma::vec a1 = Rcpp::as<arma::vec>(model_["a1"]);
  unsigned int m = a1.n_elem;
  unsigned int n;
  
  if(model_type > 0) {
    arma::vec y = Rcpp::as<arma::vec>(model_["y"]);
    n = y.n_elem;
  } else {
    arma::vec y = Rcpp::as<arma::mat>(model_["y"]);
    n = y.n_rows;
  }
  
  mcmc mcmc_run(prior_types, prior_pars, n_iter, n_burnin, n_thin, n, m,
    target_acceptance, gamma, S, true);
  
  switch (model_type) {
  case 1: {
    ung_ssm model(clone(model_), seed, Z_ind, T_ind, R_ind);
    switch (simulation_method) {
    case 1: 
      mcmc_run.pm_mcmc_psi(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    case 2:
      mcmc_run.pm_mcmc_bsf(model, end_ram, nsim_states);
      break;
    }
  } break;
  case 2: {
    ung_bsm model(clone(model_), seed);
    switch (simulation_method) {
    case 1: 
      mcmc_run.pm_mcmc_psi(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    case 2:
      mcmc_run.pm_mcmc_bsf(model, end_ram, nsim_states);
      break;
    }
  } break;
  }
  
  return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
    Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
    Rcpp::Named("counts") = mcmc_run.count_storage,
    Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
    Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
}


// [[Rcpp::export]]
Rcpp::List nongaussian_da_mcmc(const Rcpp::List& model_,
  arma::uvec prior_types, arma::mat prior_pars, unsigned int nsim_states, 
  unsigned int n_iter, unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat S,
  unsigned int seed, bool end_ram, unsigned int n_threads, bool local_approx,
  arma::vec initial_mode, unsigned int max_iter, double conv_tol,
  unsigned int simulation_method, int model_type, const arma::uvec& Z_ind,
  const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  arma::vec a1 = Rcpp::as<arma::vec>(model_["a1"]);
  unsigned int m = a1.n_elem;
  unsigned int n;
  
  if(model_type > 0) {
    arma::vec y = Rcpp::as<arma::vec>(model_["y"]);
    n = y.n_elem;
  } else {
    arma::vec y = Rcpp::as<arma::mat>(model_["y"]);
    n = y.n_rows;
  }
  
  mcmc mcmc_run(prior_types, prior_pars, n_iter, n_burnin, n_thin, n, m,
    target_acceptance, gamma, S, true);
  
  switch (model_type) {
  case 1: {
    ung_ssm model(clone(model_), seed, Z_ind, T_ind, R_ind);
    switch (simulation_method) {
    case 1: 
      mcmc_run.da_mcmc_psi(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    case 2:
      mcmc_run.da_mcmc_bsf(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    }
  } break;
  case 2: {
    ung_bsm model(clone(model_), seed);
    switch (simulation_method) {
    case 1: 
      mcmc_run.da_mcmc_psi(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    case 2:
      mcmc_run.da_mcmc_bsf(model, end_ram, nsim_states, local_approx, initial_mode, 
        max_iter, conv_tol);
      break;
    }
  } break;
  }
  
  return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
    Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
    Rcpp::Named("counts") = mcmc_run.count_storage,
    Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
    Rcpp::Named("S") = mcmc_run.S,  Rcpp::Named("posterior") = mcmc_run.posterior_storage);
}



// [[Rcpp::export]]
Rcpp::List nongaussian_is_mcmc(const Rcpp::List& model_,
  arma::uvec prior_types, arma::mat prior_pars, unsigned int nsim_states, 
  unsigned int n_iter, unsigned int n_burnin, unsigned int n_thin,
  double gamma, double target_acceptance, arma::mat S,
  unsigned int seed, bool end_ram, unsigned int n_threads, bool local_approx,
  arma::vec initial_mode, unsigned int max_iter, double conv_tol,
  unsigned int simulation_method, bool const_sim, int model_type, 
  const arma::uvec& Z_ind, const arma::uvec& T_ind, const arma::uvec& R_ind) {
  
  arma::vec a1 = Rcpp::as<arma::vec>(model_["a1"]);
  unsigned int m = a1.n_elem;
  unsigned int n;
  
  if(model_type > 0) {
    arma::vec y = Rcpp::as<arma::vec>(model_["y"]);
    n = y.n_elem;
  } else {
    arma::vec y = Rcpp::as<arma::mat>(model_["y"]);
    n = y.n_rows;
  }
  
  ung_amcmc mcmc_run(prior_types, prior_pars, n_iter, n_burnin, n_thin, n, m,
    target_acceptance, gamma, S, true);
  
  switch (model_type) {
  case 1: {
    ung_ssm model(clone(model_), seed, Z_ind, T_ind, R_ind);
    mcmc_run.approx_mcmc(model, end_ram, local_approx, initial_mode, 
      max_iter, conv_tol);
    switch (simulation_method) {
    case 1: 
      mcmc_run.is_correction_psi(model, nsim_states, const_sim, n_threads);
      break;
    case 2:
      mcmc_run.is_correction_bsf(model, nsim_states, const_sim, n_threads);
      break;
    case 3:
      break;
    }
  } break;
  case 2: {
    ung_bsm model(clone(model_), seed);
    mcmc_run.approx_mcmc(model, end_ram, local_approx, initial_mode, 
      max_iter, conv_tol);
    switch (simulation_method) {
    case 1: 
      mcmc_run.is_correction_psi(model, nsim_states, const_sim, n_threads);
      break;
    case 2:
      mcmc_run.is_correction_bsf(model, nsim_states, const_sim, n_threads);
      break;
    case 3:
      mcmc_run.is_correction_spdk(model, nsim_states, const_sim, n_threads);
      break;
    }
  } break;
  }
  
  return Rcpp::List::create(Rcpp::Named("alpha") = mcmc_run.alpha_storage,
    Rcpp::Named("theta") = mcmc_run.theta_storage.t(),
    Rcpp::Named("weights") = mcmc_run.weight_storage,
    Rcpp::Named("counts") = mcmc_run.count_storage,
    Rcpp::Named("acceptance_rate") = mcmc_run.acceptance_rate,
    Rcpp::Named("S") = mcmc_run.S, 
    Rcpp::Named("posterior") = mcmc_run.posterior_storage);
}