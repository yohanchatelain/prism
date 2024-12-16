#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#include "src/xoshiro.h"

// First undef to prevent error when re-included.
#undef HWY_TARGET_INCLUDE
// For dynamic dispatch, specify the name of the current file (unfortunately
// __FILE__ is not reliable) so that foreach_target.h can re-include it.
#define HWY_TARGET_INCLUDE "src/xoshiro_vector.cpp"
// Generates code for each enabled target by re-including this source file.
#include "hwy/foreach_target.h"

#include "hwy/base.h"
#include "hwy/highway.h"
#include "src/random-inl.h"
#include "src/target_utils.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism {
namespace scalar {
namespace xoshiro {

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

using RNG = hn::CachedXoshiro<>;
const auto seed = get_user_seed();

thread_local RNG rng(seed, gettid() % getpid());
float uniform_f32() { return rng.Uniform(); };
double uniform_f64() { return rng.Uniform(); };
std::uint64_t random_u64() { return rng(); };

} // namespace HWY_NAMESPACE

} // namespace xoshiro
} // namespace scalar

namespace vector {
namespace xoshiro {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

} // namespace HWY_NAMESPACE
} // namespace xoshiro
} // namespace vector

} // namespace prism
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace prism {
namespace scalar {
namespace xoshiro {

HWY_EXPORT(uniform_f32);
HWY_EXPORT(uniform_f64);
HWY_EXPORT(random_u64);

namespace static_dispatch {

HWY_DLLEXPORT float uniform(float) {
  return HWY_STATIC_DISPATCH(uniform_f32)();
}

HWY_DLLEXPORT double uniform(double) {
  return HWY_STATIC_DISPATCH(uniform_f64)();
}

HWY_DLLEXPORT std::uint64_t random() {
  return HWY_STATIC_DISPATCH(random_u64)();
}

} // namespace static_dispatch

namespace dynamic_dispatch {

HWY_DLLEXPORT float uniform(float) {
  return HWY_DYNAMIC_DISPATCH(uniform_f32)();
}

HWY_DLLEXPORT double uniform(double) {
  return HWY_DYNAMIC_DISPATCH(uniform_f64)();
}

HWY_DLLEXPORT std::uint64_t random() {
  return HWY_DYNAMIC_DISPATCH(random_u64)();
}

} // namespace dynamic_dispatch

} // namespace xoshiro
} // namespace scalar

} // namespace prism

#endif // HWY_ONCE