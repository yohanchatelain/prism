#ifndef __PRSIM_EFT_H__
#define __PRSIM_EFT_H__

#include <cmath>

#include "src/debug.h"

// fast two sum if |a| > |b|
// a and b are not nan or inf
template <typename T> void fasttwosum(T a, T b, T &sigma, T &tau) {
  if (std::abs(a) < std::abs(b))
    std::swap(a, b);
  sigma = a + b;
  T z = sigma - a;
  tau = (a - (sigma - z)) + (b - z);
  debug_print("twosum(%.13a, %.13a) = %.13a, %.13a\n", a, b, sigma, tau);
}

// twosum
template <typename T> inline void twosum(T a, T b, T &sigma, T &tau) {
  sigma = a + b;
  T ap = sigma - b;
  T bp = sigma - ap;
  T da = a - ap;
  T db = b - bp;
  tau = da + db;
  debug_print("twosum(%.13a, %.13a) = %.13a, %.13a\n", a, b, sigma, tau);
}

template <typename T> inline void twoprodfma(T a, T b, T &sigma, T &tau) {
  sigma = a * b;
  tau = std::fma(a, b, -sigma);
  debug_print("twoprodfma(%.13a, %.13a) = %.13a, %.13a\n", a, b, sigma, tau);
}

#endif // __PRSIM_EFT_H__