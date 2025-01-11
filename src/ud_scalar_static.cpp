
#include "src/debug.h"
// #include "src/ud_scalar.h"

// Generates code for every target that this compiler can support.
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/ud_scalar_static.cpp" // this file
#include "hwy/foreach_target.h" // must come before highway.h

// clang-format off
#include "hwy/highway.h"
#include "src/ud_scalar-inl.h"
// clang-format on

#if HWY_ONCE

namespace prism::ud::scalar::static_dispatch {

HWY_EXPORT(addf32);
HWY_EXPORT(subf32);
HWY_EXPORT(mulf32);
HWY_EXPORT(divf32);
HWY_EXPORT(sqrtf32);
HWY_EXPORT(fmaf32);

HWY_EXPORT(addf64);
HWY_EXPORT(subf64);
HWY_EXPORT(mulf64);
HWY_EXPORT(divf64);
HWY_EXPORT(sqrtf64);
HWY_EXPORT(fmaf64);

/* static dispatch */
float addf32(float a, float b) { return HWY_STATIC_DISPATCH(addf32)(a, b); }
float subf32(float a, float b) { return HWY_STATIC_DISPATCH(subf32)(a, b); }
float mulf32(float a, float b) { return HWY_STATIC_DISPATCH(mulf32)(a, b); }
float divf32(float a, float b) { return HWY_STATIC_DISPATCH(divf32)(a, b); }
float sqrtf32(float a) { return HWY_STATIC_DISPATCH(sqrt)(a); }
float fmaf32(float a, float b, float c) {
  return HWY_STATIC_DISPATCH(fma)(a, b, c);
}

double addf64(double a, double b) { return HWY_STATIC_DISPATCH(addf64)(a, b); }
double subf64(double a, double b) { return HWY_STATIC_DISPATCH(subf64)(a, b); }
double mulf64(double a, double b) { return HWY_STATIC_DISPATCH(mulf64)(a, b); }
double divf64(double a, double b) { return HWY_STATIC_DISPATCH(divf64)(a, b); }
double sqrtf64(double a) { return HWY_STATIC_DISPATCH(sqrtf64)(a); }
double fmaf64(double a, double b, double c) {
  return HWY_STATIC_DISPATCH(fmaf64)(a, b, c);
}

} // namespace prism::ud::scalar::static_dispatch

#endif // HWY_ONCE
