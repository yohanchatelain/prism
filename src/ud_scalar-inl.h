#include "src/debug.h"
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

#include "src/utils.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::ud::scalar::HWY_NAMESPACE {

namespace rng = prism::scalar::xoshiro::HWY_NAMESPACE;

template <typename T> HWY_FLATTEN auto round(T a) -> T {
  debug_start();
  if (a == 0) {
    debug_end();
    return a;
  }
  debug_print("a        = %.13a\n", a);
  prism::utils::binaryN<T> a_bits = {.f = a};
  using U = decltype(a_bits.u);
  constexpr U one = 1;
#ifdef PRISM_RANDOM_FULLBITS
  const auto rand = rng::randombit(U{});
  // get 1 or -1
  a_bits.i += 1 - (rand << one);
#else
  const auto rand = rng::random();
  // get the last bit of the random number to get -1 or 1
  a_bits.i += 1 - ((rand & one) << one);
#endif
  debug_print("rand     = 0x%02x\n", rand);
  debug_print("round(a) = %.13a\n", a);
  debug_end();
  return a_bits.f;
}

template <typename T> HWY_FLATTEN auto add(T a, T b) -> T {
  return round(a + b);
}
template <typename T> HWY_FLATTEN auto sub(T a, T b) -> T {
  return round(a - b);
}
template <typename T> HWY_FLATTEN auto mul(T a, T b) -> T {
  return round(a * b);
}
template <typename T> HWY_FLATTEN auto div(T a, T b) -> T {
  return round(a / b);
}
template <typename T> HWY_FLATTEN auto sqrt(T a) -> T {
  return round(std::sqrt(a));
}
template <typename T> HWY_FLATTEN auto fma(T a, T b, T c) -> T {
  return round(std::fma(a, b, c));
}

/* binary32 */
HWY_FLATTEN auto addf32(float a, float b) -> float { return round(a + b); }
HWY_FLATTEN auto subf32(float a, float b) -> float { return round(a - b); }
HWY_FLATTEN auto mulf32(float a, float b) -> float { return round(a * b); }
HWY_FLATTEN auto divf32(float a, float b) -> float { return round(a / b); }
HWY_FLATTEN auto sqrtf32(float a) -> float { return round(std::sqrt(a)); }
HWY_FLATTEN auto fmaf32(float a, float b, float c) -> float {
  return round(std::fma(a, b, c));
}

/* binary64 */
HWY_FLATTEN auto addf64(double a, double b) -> double { return round(a + b); }
HWY_FLATTEN auto subf64(double a, double b) -> double { return round(a - b); }
HWY_FLATTEN auto mulf64(double a, double b) -> double { return round(a * b); }
HWY_FLATTEN auto divf64(double a, double b) -> double { return round(a / b); }
HWY_FLATTEN auto sqrtf64(double a) -> double { return round(std::sqrt(a)); }
HWY_FLATTEN auto fmaf64(double a, double b, double c) -> double {
  return round(std::fma(a, b, c));
}

} // namespace prism::ud::scalar::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#endif // PRISM_UD_SCALAR_INL_H_