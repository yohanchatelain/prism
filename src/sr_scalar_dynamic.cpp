
#include "src/debug.h"
// #include "src/ud_scalar.h"

// Generates code for every target that this compiler can support.
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/sr_scalar_dynamic.cpp" // this file
#include "hwy/foreach_target.h" // must come before highway.h

// clang-format off
#include "hwy/highway.h"
#include "src/sr_scalar-inl.h"
// clang-format on

#if HWY_ONCE

namespace prism::sr::scalar::dynamic_dispatch {

namespace {
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
} // namespace

/* dynamic dispatch */
auto addf32(float a, float b) -> float {
  return HWY_DYNAMIC_DISPATCH(addf32)(a, b);
}
auto subf32(float a, float b) -> float {
  return HWY_DYNAMIC_DISPATCH(subf32)(a, b);
}
auto mulf32(float a, float b) -> float {
  return HWY_DYNAMIC_DISPATCH(mulf32)(a, b);
}
auto divf32(float a, float b) -> float {
  return HWY_DYNAMIC_DISPATCH(divf32)(a, b);
}
auto sqrtf32(float a) -> float { return HWY_DYNAMIC_DISPATCH(sqrtf32)(a); }
auto fmaf32(float a, float b, float c) -> float {
  return HWY_DYNAMIC_DISPATCH(fmaf32)(a, b, c);
}

auto addf64(double a, double b) -> double {
  return HWY_DYNAMIC_DISPATCH(addf64)(a, b);
}
auto subf64(double a, double b) -> double {
  return HWY_DYNAMIC_DISPATCH(subf64)(a, b);
}
auto mulf64(double a, double b) -> double {
  return HWY_DYNAMIC_DISPATCH(mulf64)(a, b);
}
auto divf64(double a, double b) -> double {
  return HWY_DYNAMIC_DISPATCH(divf64)(a, b);
}
auto sqrtf64(double a) -> double { return HWY_DYNAMIC_DISPATCH(sqrtf64)(a); }
auto fmaf64(double a, double b, double c) -> double {
  return HWY_DYNAMIC_DISPATCH(fmaf64)(a, b, c);
}

} // namespace prism::sr::scalar::dynamic_dispatch

#endif // HWY_ONCE
