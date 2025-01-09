#include "src/debug.h"
#include "src/eft.h"
#include "src/utils.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#if defined(PRISM_SR_SCALAR_INL_H) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_SR_SCALAR_INL_H
#undef PRISM_SR_SCALAR_INL_H
#else
#define PRISM_SR_SCALAR_INL_H
#endif

#include "src/random-inl.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::sr::scalar::HWY_NAMESPACE {

namespace rng = prism::scalar::xoshiro::HWY_NAMESPACE;

template <typename T> auto isnumber(const T a, const T b) -> bool {
  using U = typename prism::utils::IEEE754<T>::U;
  // fast check for a or b is not 0, inf or nan
  debug_start();
  constexpr auto naninf_mask = prism::utils::IEEE754<T>::inf_nan_mask;
  prism::utils::binaryN<T> a_bits = {.f = a};
  prism::utils::binaryN<T> b_bits = {.f = b};
  const U a_uint = a_bits.u;
  const U b_uint = b_bits.u;
  const bool ret =
      ((a_uint != 0) and ((a_uint & naninf_mask) != naninf_mask)) and
      ((b_uint != 0) and ((b_uint & naninf_mask) != naninf_mask));
  debug_print("a_bits = 0x%016x\n", a_uint);
  debug_print("b_bits = 0x%016x\n", b_uint);
  debug_print("0x%016x & 0x%016x = 0x%016x\n", a_uint, naninf_mask,
              a_uint & naninf_mask);
  debug_print("0x%016x & 0x%016x = 0x%016x\n", b_uint, naninf_mask,
              b_uint & naninf_mask);
  debug_print("isnumber(%.13a, %.13a) = %d\n", a, b, ret);
  debug_end();
  return ret;
}

template <typename T> inline auto round(const T sigma, const T tau) -> T {
  using prism::utils::get_exponent;
  using prism::utils::get_predecessor_abs;
  using prism::utils::IEEE754;
  using prism::utils::pow2;
  debug_start();
  if (tau == 0) {
    debug_end();
    return 0;
  }
  constexpr int32_t mantissa = IEEE754<T>::mantissa;
  const bool sign_tau = tau < 0;
  const bool sign_sigma = sigma < 0;
  const int32_t eta = (sign_tau != sign_sigma)
                          ? get_exponent(get_predecessor_abs(sigma))
                          : get_exponent(sigma);
  const T ulp = (sign_tau ? -1 : 1) * pow2<T>(eta - mantissa);
  const T z = rng::uniform(T{});
  const T pi = ulp * z;
  const T rnd = (std::abs(tau + pi) >= std::abs(ulp)) ? ulp : 0;

  debug_print("z     = %+.13a\n", z);
  debug_print("sigma = %+.13a\n", sigma);
  debug_print("tau   = %+.13a\n", tau);
  debug_print("eta   = %d\n", eta);
  debug_print("pi    = %+.13a\n", pi);
  debug_print("tau+pi= %+.13a\n", tau + pi);
  debug_print("ulp   = %+.13a\n", ulp);
  debug_print("sr_round(%+.13a, %+.13a, %+.13a) = %+.13a\n", sigma, tau, z,
              rnd);
  debug_end();
  return rnd;
}

template <typename T> inline auto add(const T a, const T b) -> T {
  debug_start();
  if (not isnumber(a, b)) {
    debug_end();
    return a + b;
  }
  T tau;
  T sigma;
  twosum(a, b, sigma, tau);
  const T rnd = round(sigma, tau);
  debug_print("sr_add(%+.13a, %+.13a) = %+.13a + %+.13a\n", a, b, sigma, rnd);
  debug_end();
  return sigma + rnd;
}

template <typename T> inline auto sub(T a, T b) -> T { return add(a, -b); }

template <typename T> inline auto mul(T a, T b) -> T {
  debug_start();
  if (not isnumber(a, b)) {
    debug_end();
    return a * b;
  }
  T tau;
  T sigma;
  twoprodfma(a, b, sigma, tau);
  const T rnd = round(sigma, tau);
  debug_end();
  return sigma + rnd;
}

template <typename T> inline auto div(const T a, const T b) -> T {
  debug_start();
  if (not isnumber(a, b)) {
    debug_end();
    return a / b;
  }
  const T sigma = a / b;
  debug_print("sigma = %+.13a / %+.13a = %+.13a\n", a, b, sigma);

  const T tau = std::fma(-sigma, b, a) / b;

  debug_print("-sigma * b + a = %+.13a * %+.13a + %+.13a = %+.13a\n", -sigma, b,
              a, std::fma(-sigma, b, a));
  debug_print("tau = (-%+.13a * %+.13a + %+.13a) / %+.13a\n", -sigma, b, a, b);

  const T rnd = round(sigma, tau);
  debug_print("sr_div(%+.13a, %+.13a) = %+.13a + %+.13a\n", a, b, sigma, tau);
  debug_end();
  return sigma + rnd;
}

template <typename T> inline auto sqrt(const T a) -> T {
  const T sigma = std::sqrt(a);
  if (not std::isfinite(a)) {
    return sigma;
  }
  const T tau_p = std::fma(-sigma, sigma, a);
  const T tau = tau_p / (2 * sigma);
  const T rnd = round(sigma, tau);
  return sigma + rnd;
}

/*
"Exact and Approximated error of the FMA"
Sylvie Boldo, Jean-Michel Muller
---
Algorithm 5 (ErrFmaNearest):
  r1 = ◦(ax + y)
  (u1, u2) = Fast2Mult(a, x)
  (α1, α2) = 2Sum(y, u2)
  (β1, β2) = 2Sum(u1, α1)
  γ = ◦(◦(β1 − r1) + β2)
  r2 = ◦(γ + α2)
*/
template <typename T> inline auto fma(const T a, const T b, const T c) -> T {
  if (not std::isfinite(a) or not std::isfinite(b) or not std::isfinite(c)) {
    return std::fma(a, b, c);
  }
  debug_start();
  T u1;
  T u2;
  T alpha1;
  T alpha2;
  T beta1;
  T beta2;
  T gamma;
  T r1;
  T r2;
  r1 = std::fma(a, b, c);
  twoprodfma(a, b, u1, u2);
  twosum(c, u2, alpha1, alpha2);
  twosum(u1, alpha1, beta1, beta2);
  gamma = (beta1 - r1) + beta2;
  r2 = gamma + alpha2;
  const T rnd = round(r1, r2);
  debug_print("sr_fma(%+.13a, %+.13a, %+.13a) = %+.13a + %+.13a\n", a, b, c, r1,
              r2);
  debug_end();
  return r1 + rnd;
}

/* binary32 */
inline auto addf32(float a, float b) -> float { return add(a, b); }
inline auto subf32(float a, float b) -> float { return sub(a, b); }
inline auto mulf32(float a, float b) -> float { return mul(a, b); }
inline auto divf32(float a, float b) -> float { return div(a, b); }
inline auto sqrtf32(float a) -> float { return sqrt(a); }
inline auto fmaf32(float a, float b, float c) -> float { return fma(a, b, c); }

/* binary64 */
inline auto addf64(double a, double b) -> double { return add(a, b); }
inline auto subf64(double a, double b) -> double { return sub(a, b); }
inline auto mulf64(double a, double b) -> double { return mul(a, b); }
inline auto divf64(double a, double b) -> double { return div(a, b); }
inline auto sqrtf64(double a) -> double { return sqrt(a); }
inline auto fmaf64(double a, double b, double c) -> double {
  return fma(a, b, c);
}

} // namespace prism::sr::scalar::HWY_NAMESPACE

HWY_AFTER_NAMESPACE();

#endif // PRISM_SR_SCALAR_INL_H
