#include "ng_bsm.h"

// from List
ng_bsm::ng_bsm(const List model, unsigned int seed, bool log_space) :
  ngssm(model, seed), slope(model["slope"]), seasonal(model["seasonal"]),
  noise(model["noise"]),
  fixed(as<arma::uvec>(model["fixed"])), level_est(fixed(0) == 0),
  slope_est(slope && fixed(1) == 0), seasonal_est(seasonal && fixed(2) == 0),
  log_space(log_space) {
}


//general constructor
ng_bsm::ng_bsm(arma::vec y, arma::mat Z, arma::cube T,
  arma::cube R, arma::vec a1, arma::mat P1, arma::vec phi, bool slope, bool seasonal,
  bool noise, arma::uvec fixed, arma::mat xreg, arma::vec beta, unsigned int distribution,
  unsigned int seed, bool log_space) :
  ngssm(y, Z, T, R, a1, P1, phi, xreg, beta, distribution, seed),
  slope(slope), seasonal(seasonal), noise(noise), fixed(fixed), level_est(fixed(0) == 0),
  slope_est(slope && fixed(1) == 0), seasonal_est(seasonal && fixed(2) == 0),
  log_space(log_space) {
}

//without log_space
ng_bsm::ng_bsm(arma::vec y, arma::mat Z, arma::cube T,
  arma::cube R, arma::vec a1, arma::mat P1, arma::vec phi, bool slope, bool seasonal,
  bool noise, arma::uvec fixed, arma::mat xreg, arma::vec beta, unsigned int distribution,
  unsigned int seed) :
  ngssm(y, Z, T, R, a1, P1, phi, xreg, beta, distribution, seed),
  slope(slope), seasonal(seasonal), noise(noise), fixed(fixed), level_est(fixed(0) == 0),
  slope_est(slope && fixed(1) == 0), seasonal_est(seasonal && fixed(2) == 0),
  log_space(false) {
}

double ng_bsm::proposal(const arma::vec& theta, const arma::vec& theta_prop) {
  double q = 0.0;
  if(log_space && (sum(fixed) < 3 || noise)) {
    if (level_est) {
      q += theta_prop(0) - theta(0);
    }
    if (slope_est) {
      q += theta_prop(level_est) - theta(level_est);
    }
    if (seasonal_est) {
      q += theta_prop(level_est + slope_est) - theta(level_est + slope_est);
    }
    if (noise) {
      q += theta_prop(level_est + slope_est + seasonal_est) - theta(level_est + slope_est + seasonal_est);
    }
  }
  return q;
}

void ng_bsm::update_model(arma::vec theta) {

  if (sum(fixed) < 3 || noise) {
    if (log_space) {
      theta.subvec(0, theta.n_elem - xreg.n_cols - (distribution == 3) - 1) =
        exp(theta.subvec(0, theta.n_elem - xreg.n_cols - (distribution == 3) - 1));
    }
    // sd_level
    if (level_est) {
      R(0, 0, 0) = theta(0);
    }
    // sd_slope
    if (slope_est) {
      R(1, level_est, 0) = theta(level_est);
    }
    // sd_seasonal
    if (seasonal_est) {
      R(1 + slope, level_est + slope_est, 0) =
        theta(level_est + slope_est);
    }
    if(noise) {
      R(m - 1, level_est + slope_est + seasonal_est, 0) =
        theta(level_est + slope_est + seasonal_est);
      P1(m - 1, m - 1) = std::pow(theta(level_est + slope_est + seasonal_est), 2);
    }
    compute_RR();
  }
  if(xreg.n_cols > 0) {
    beta = theta.subvec(theta.n_elem - xreg.n_cols - (distribution == 3),
      theta.n_elem - 1 - (distribution == 3));
    compute_xbeta();
  }
  if(distribution == 3) {
    phi.fill(exp(theta(theta.n_elem - 1)));
  }

}

arma::vec ng_bsm::get_theta(void) {

  unsigned int npar = level_est + slope_est + seasonal_est + noise +
    xreg.n_cols + (distribution == 3);
  Rcout<<npar<<std::endl;
  arma::vec theta(npar);
  Rcout<<R<<std::endl;
  Rcout<<fixed<<std::endl;
  Rcout<<level_est<<std::endl;
  Rcout<<slope_est<<std::endl;
  Rcout<<seasonal_est<<std::endl;
  if (sum(fixed) < 3 || noise) {
    // sd_level
    if (level_est) {
      Rcout<<theta<<std::endl;
      theta(0) = R(0, 0, 0);
    }
    // sd_slope
    if (slope_est) {
      Rcout<<theta<<std::endl;
      theta(level_est) = R(1, level_est, 0);
    }
    // sd_seasonal
    if (seasonal_est) {
      Rcout<<theta<<std::endl;
      theta(level_est + slope_est) =
        R(1 + slope, level_est + slope_est, 0);
    }
    if (noise) {
      Rcout<<theta<<std::endl;
      theta(level_est + slope_est + seasonal_est) =
        R(m - 1, level_est + slope_est + seasonal_est, 0);
    }
    if (log_space) {
      Rcout<<theta<<std::endl;
      theta.subvec(0, theta.n_elem - xreg.n_cols - (distribution == 3) - 1) =
        log(theta.subvec(0, theta.n_elem - xreg.n_cols - (distribution == 3) - 1));
    }
  }
  Rcout<<xreg<<std::endl;
  if(xreg.n_cols > 0) {
    theta.subvec(theta.n_elem - xreg.n_cols - (distribution == 3),
      theta.n_elem - 1 - (distribution == 3)) = beta;
  }

  if(distribution == 3) {
    theta(theta.n_elem - 1) = log(phi(0));
  }
  return theta;
}

// from approximating model
double ng_bsm::log_likelihood(bool demean) {

  double logLik = 0;
  arma::vec at = a1;
  arma::mat Pt = P1;

  if (demean && xreg.n_cols > 0) {
    for (unsigned int t = 0; t < n; t++) {
      logLik += uv_filter(y(t) - xbeta(t), Z.unsafe_col(0), HH(t),
        T.slice(0), RR.slice(0), at, Pt, zero_tol);
    }
  } else {
    for (unsigned int t = 0; t < n; t++) {
      logLik += uv_filter(y(t), Z.unsafe_col(0), HH(t),
        T.slice(0), RR.slice(0), at, Pt, zero_tol);
    }
  }

  return logLik;
}

