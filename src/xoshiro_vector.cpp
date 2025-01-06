#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <unistd.h>

#include <cstdlib>
#include <execinfo.h>

// First undef to prevent error when re-included.
#undef HWY_TARGET_INCLUDE
// For dynamic dispatch, specify the name of the current file (unfortunately
// __FILE__ is not reliable) so that foreach_target.h can re-include it.
#define HWY_TARGET_INCLUDE "src/xoshiro_vector.cpp"
// Generates code for each enabled target by re-including this source file.
#include "hwy/foreach_target.h"

#include "hwy/aligned_allocator.h"
#include "hwy/base.h"
#include "hwy/highway.h"
#include "src/random-inl.h"
#include "src/target_utils.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::scalar::xoshiro {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
using RNG = hn::CachedXoshiro<1024 * 8>;
thread_local RNG *rng = nullptr;

void debug(const char fmt[], ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Scalar] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

void _init_rng() {
#if PRISM_RNG_DEBUG
  // WARNING: Do not use c++ ostream in the constructor as some of its internal
  // objects are not initialized yet. Use fprintf instead.
  debug("Initializing rng\n");
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
  assert(rng == nullptr);
#endif
  const auto seed = get_user_seed();
  const auto tid = gettid() % getpid();
  rng = new RNG(seed, tid);
#if PRISM_RNG_DEBUG
  debug("rng allocated at %p\n", (void *)rng);
#endif
}

RNG *get_rng() {
  if (rng == nullptr) {
    _init_rng();
  }
  return rng;
}

float uniform(float) { return static_cast<float>(get_rng()->Uniform()); }

double uniform(double) { return get_rng()->Uniform(); }

std::uint64_t random() { return get_rng()->operator()(); }

} // namespace HWY_NAMESPACE
} // namespace prism::scalar::xoshiro

namespace prism::vector::xoshiro {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
using RNG = hn::VectorXoshiro;
thread_local RNG *rng = nullptr;

void debug(const char fmt[], ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Vector] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

void _init_rng() {
#if PRISM_RNG_DEBUG
  // WARNING: Do not use c++ ostream in the constructor as some of its internal
  // objects are not initialized yet. Use fprintf instead.
  debug("Initializing rng\n");
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
  assert(rng == nullptr);
#endif
  const auto seed = get_user_seed();
  const auto tid = gettid() % getpid();
  rng = new RNG(seed, tid);
  assert(rng != nullptr);
#if PRISM_RNG_DEBUG
  debug("rng allocated at %p\n", (void *)rng);
#endif
}

HWY_API RNG *get_rng() {
  if (HWY_UNLIKELY(rng == nullptr)) {
    _init_rng();
  }
  assert(rng != nullptr);
  return rng;
}

HWY_FLATTEN hn::Vec<hn::ScalableTag<float>> uniform(float f) {
  return get_rng()->Uniform(f);
}

HWY_FLATTEN hn::Vec<hn::ScalableTag<double>> uniform(double d) {
  return get_rng()->Uniform(d);
}

HWY_FLATTEN hn::Vec<hn::ScalableTag<std::uint32_t>> random(std::uint32_t u) {
  return get_rng()->operator()(u);
}

HWY_FLATTEN hn::Vec<hn::ScalableTag<std::uint64_t>> random(std::uint64_t u) {
  return get_rng()->operator()(u);
}

} // namespace HWY_NAMESPACE
} // namespace prism::vector::xoshiro

HWY_AFTER_NAMESPACE();

#if HWY_ONCE

void debug(const char fmt[], ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Xoshiro] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

__attribute__((constructor)) void _init() {
  hwy::GetChosenTarget().Update(hwy::SupportedTargets());
#if PRISM_RNG_DEBUG
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
#endif
}
#endif // HWY_ONCE
