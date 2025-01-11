#ifndef __PRISM_UTILS_H__
#define __PRISM_UTILS_H__

#include <cstdint>
#include <cstring>
#include <string>

#include "src/debug.h"

#ifdef __clang__
using Float128 = __float128;
#elif defined(__GNUC__) || defined(__GNUG__)
typedef _Float128 Float128;
#else
#error "Unsupported compiler. Please use GCC or Clang."
#endif

namespace prism::utils {

template <typename T> struct IEEE754 {};

// specialize IEEE754 for float and double

template <> struct IEEE754<float> {
  using I = std::int32_t;
  using U = std::uint32_t;
  using H = double;
  static constexpr I sign = 1;
  static constexpr I exponent = 8;
  static constexpr I mantissa = 23;
  static constexpr I precision = 24;
  static constexpr I precision10 = 7;
  static constexpr float ulp = 0x1.0p-23F;
  static constexpr float half_ulp = 0x1.0p-24F;
  static constexpr I bias = 127;
  static constexpr U exponent_mask = 0x7F8;
  static constexpr U exponent_mask_scaled = 0x7f800000;
  static constexpr float max_normal = 0x1.fffffep127F;
  static constexpr float min_normal = 0x1.0p-126F;
  static constexpr float min_subnormal = 0x1.0p-149F;
  static constexpr I min_exponent = -126;
  static constexpr I max_exponent = 127;
  static constexpr I min_exponent_subnormal = -149;
  static constexpr U inf_nan_mask = 0x7F800000;
  static constexpr const char *format = "%.6a";
};

template <> struct IEEE754<double> {
  using I = std::int64_t;
  using U = std::uint64_t;
  using H = Float128;
  static constexpr I sign = 1;
  static constexpr I exponent = 11;
  static constexpr I mantissa = 52;
  static constexpr I precision = 53;
  static constexpr I precision10 = 17;
  static constexpr double ulp = 0x1.0p-52;
  static constexpr double half_ulp = 0x1.0p-53;
  static constexpr I bias = 1023;
  static constexpr U exponent_mask = 0x7FF;
  static constexpr U exponent_mask_scaled = 0x7ff0000000000000ULL;
  static constexpr double max_normal = 0x1.fffffffffffffp1023;
  static constexpr double min_normal = 0x1.0p-1022;
  static constexpr double min_subnormal = 0x1.0p-1074;
  static constexpr I min_exponent = -1022;
  static constexpr I max_exponent = 1023;
  static constexpr I min_exponent_subnormal = -1074;
  static constexpr U inf_nan_mask = 0x7FF0000000000000;
  static constexpr const char *format = "%.13a";
};

template <typename T> union binaryN {};

template <> union binaryN<float> {
  float f;
  uint32_t u;
  int32_t i;
};

template <> union binaryN<double> {
  double f;
  uint64_t u;
  int64_t i;
};

// Implement other functions (get_exponent, predecessor, abs, pow2, etc.) using
// templates
// "Emulating round-to-nearest ties-to-zero "augmented” floating-point
// operations using round-to-nearest ties-to-even arithmetic"
// Sylvie Boldo, Christoph Q. Lauter, Jean-Michel Muller
// ALGORITHM 4: MyulpH(𝑎): Computes
// sign(𝑎) · pred(|𝑎|) and sign(𝑎) · ulp𝐻 (𝑎) for |𝑎| > 2𝑒min .
// Uses the FP constant 𝜓 = 1 − 2^{−𝑝} .
// 𝑧 ← RN𝑒 (𝜓𝑎) (= pred(|a|))
// 𝛿 ← RN𝑒 (𝑎 − 𝑧)
// return (z,𝛿)
template <typename T> auto get_predecessor_abs(T a) -> T {
  constexpr auto half_ulp = IEEE754<T>::ulp / 2;
  const T phi = 1.0 - half_ulp;
  const T z = a * phi;
  return z;
}

template <typename T, typename U = typename IEEE754<T>::U>
auto get_unbiased_exponent(T a) -> U {
  if (a == 0) {
    return 0;
  }
  constexpr U mantissa = IEEE754<T>::mantissa;
  constexpr U exponent_mask = IEEE754<T>::exponent_mask;
  U exp;
  std::memcpy(&exp, &a, sizeof(T));
  exp = ((exp >> mantissa) & exponent_mask);
  return exp;
}

template <typename T, typename I = typename IEEE754<T>::I>
auto get_exponent(T a) -> I {
  debug_start();
  if (a == 0) {
    debug_end();
    return 0;
  }
  using U = typename IEEE754<T>::U;
  constexpr I bias = IEEE754<T>::bias;
  constexpr I mantissa = IEEE754<T>::mantissa;
  constexpr U exponent_mask = IEEE754<T>::exponent_mask_scaled;
  debug_print("a = %+.13a\n", a);
  debug_print("bias = %d\n", bias);
  debug_print("mantissa = %d\n", mantissa);
  debug_print("exponent_mask = %d\n", exponent_mask);
  I *a_bits = reinterpret_cast<I *>(&a);
  debug_print("a = 0x%016x\n", a_bits);
  const auto raw_exp = ((*a_bits) & exponent_mask) >> mantissa;
  debug_print("raw exponent = %d\n", raw_exp);
  const I exp = raw_exp - bias;
  debug_print("get_exponent(%.13a) = %d\n", a, exp);
  debug_end();
  return exp;
}

template <typename T> auto pow2(int n) -> T {
  // if n <= min_exponent, take into account precision loss due to subnormal
  // numbers
  using U = typename IEEE754<T>::I;
  constexpr auto mantissa = IEEE754<T>::mantissa;
  constexpr auto min_exponent = IEEE754<T>::min_exponent;

  const auto is_subnormal = n < min_exponent;
  const int precision_loss = (is_subnormal) ? min_exponent - n : 0;
  n = (is_subnormal) ? 1 : n;
  T res = (is_subnormal) ? 0 : 1;
  U i;
  std::memcpy(&i, &res, sizeof(U));
  i += static_cast<U>(n) << (mantissa - precision_loss);
  std::memcpy(&res, &i, sizeof(T));

  debug_print("pow2(%d) = %.13a\n", n, res);

  return res;
}

// TODO(yohan): finish to implement this function, bug in rounding logic
template <typename T> auto add_round_odd(T a, T b) -> T {
  // return addition with rounding to odd
  // https://www.lri.fr/~melquion/doc/08-tc.pdf
  T x;
  T e;
  twosum(a, b, &x, &e);
  union {
    T value;
    typename IEEE754<T>::U bits;
  } u;
  u.value = x;
  return (e == 0 || (u.bits & 1)) ? x : x + 1;
}

auto predecessor_float(float a) -> float;
auto predecessor_double(double a) -> double;

auto get_exponent_float(float a) -> int32_t;
auto get_exponent_double(double a) -> int64_t;

auto pow2_float(int32_t n) -> float;
auto pow2_double(int64_t n) -> double;

} // namespace prism::utils

#endif // __PRISM_UTILS_H__
