#include "src/debug.h"
#include "src/eft.h"
#include "src/utils.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#if defined(PRISM_UD_SCALAR_INL_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_UD_SCALAR_INL_H_
#undef PRISM_UD_SCALAR_INL_H_
#else
#define PRISM_UD_SCALAR_INL_H_
#endif

#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::ud::scalar::HWY_NAMESPACE {

namespace rng = prism::scalar::xoshiro::HWY_NAMESPACE;

template <typename T> auto round(T a) -> T {
  debug_start();
  if (a == 0) {
    debug_end();
    return a;
  }
  debug_print("a        = %.13a\n", a);
  using I = typename prism::utils::IEEE754<T>::I;
  I a_bits = *reinterpret_cast<I *>(&a);
  std::uint64_t rand = rng::random();
  debug_print("rand     = 0x%02x\n", rand);
  // get the last bit of the random number to get -1 or 1
  a_bits += 1 - ((rand & 1) << 1);
  a = *reinterpret_cast<T *>(&a_bits);
  debug_print("round(a) = %.13a\n", a);
  debug_end();
  return a;
} // namespace prism::scalar::xoroshiro256plus::static_dispatch

template <typename T> T add(T a, T b) { return round(a + b); }
template <typename T> T sub(T a, T b) { return round(a - b); }
template <typename T> T mul(T a, T b) { return round(a * b); }
template <typename T> T div(T a, T b) { return round(a / b); }
template <typename T> T sqrt(T a) { return round(std::sqrt(a)); }
template <typename T> T fma(T a, T b, T c) { return round(std::fma(a, b, c)); }

/* binary32 */
inline float addf32(float a, float b) { return round(a + b); }
inline float subf32(float a, float b) { return round(a - b); }
inline float mulf32(float a, float b) { return round(a * b); }
inline float divf32(float a, float b) { return round(a / b); }
inline float sqrtf32(float a) { return round(std::sqrt(a)); }
inline float fmaf32(float a, float b, float c) {
  return round(std::fma(a, b, c));
}

/* binary64 */
inline double addf64(double a, double b) { return round(a + b); }
inline double subf64(double a, double b) { return round(a - b); }
inline double mulf64(double a, double b) { return round(a * b); }
inline double divf64(double a, double b) { return round(a / b); }
inline double sqrtf64(double a) { return round(std::sqrt(a)); }
inline double fmaf64(double a, double b, double c) {
  return round(std::fma(a, b, c));
}

} // namespace prism::ud::scalar::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#endif // PRISM_UD_SCALAR_INL_H_