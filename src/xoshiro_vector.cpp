#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <cstdint>
#endif
#include <stdlib.h>
#include <unistd.h>

#include <cstdlib>
#include <execinfo.h>
#include <memory>

// First undef to prevent error when re-included.
#undef HWY_TARGET_INCLUDE
// For dynamic dispatch, specify the name of the current file (unfortunately
// __FILE__ is not reliable) so that foreach_target.h can re-include it.
#define HWY_TARGET_INCLUDE "src/xoshiro_vector.cpp"
// Generates code for each enabled target by re-including this source file.
#include "hwy/foreach_target.h" // NOLINT IWYU pragma: keep

#include "hwy/base.h"
#include "hwy/highway.h"
#include "src/random-inl.h"
#include "src/target_utils.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::scalar::xoshiro::HWY_NAMESPACE {

namespace internal {
thread_local std::unique_ptr<RNG> rng = nullptr;

void debug(const char *fmt, ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Scalar] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

void init_rng(const std::uint64_t seed = get_user_seed(),
              const std::uint64_t tid = gettid() % getpid()) {
#if PRISM_RNG_DEBUG
  // WARNING: Do not use c++ ostream in the constructor as some of its internal
  // objects are not initialized yet. Use fprintf instead.
  debug("Initializing rng\n");
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
  assert(rng == nullptr);
#endif
  rng = std::make_unique<RNG>(seed, tid);
#if PRISM_RNG_DEBUG
  debug("rng allocated at %p\n", (void *)rng.get());
#endif
}

auto get_rng() -> RNG * {
  if (rng == nullptr) {
    init_rng();
  }
  return rng.get();
}
}; // namespace internal

/* API */

HWY_FLATTEN auto uniform(float /*unused*/) -> float {
  return static_cast<float>(internal::get_rng()->Uniform());
}

HWY_FLATTEN auto uniform(double /*unused*/) -> double {
  return internal::get_rng()->Uniform();
}

HWY_FLATTEN auto random() -> std::uint64_t {
  return internal::get_rng()->operator()();
}

HWY_FLATTEN auto randombit(std::uint64_t /* unused */) -> std::uint64_t {
  thread_local std::uint64_t u = 0;
  thread_local std::size_t idx = UINT64_WIDTH;
  if (idx == UINT64_WIDTH) {
    u = random();
    idx = 0;
  }
  return (u >> idx++) & UINT64_C(1);
}

HWY_FLATTEN auto randombit(std::uint32_t /* unused */) -> std::uint32_t {
  thread_local std::uint64_t u = 0;
  thread_local std::size_t idx = UINT64_WIDTH;
  if (idx == UINT64_WIDTH) {
    u = random();
    idx = 0;
  }
  return (u >> idx++) & UINT64_C(1);
}

} // namespace prism::scalar::xoshiro::HWY_NAMESPACE

namespace prism::vector::xoshiro::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;
namespace internal {
thread_local std::unique_ptr<RNG> rng = nullptr;

void debug(const char *fmt, ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Vector] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

void init_rng(const std::uint64_t seed = get_user_seed(),
              const std::uint64_t tid = gettid() % getpid()) {
#if PRISM_RNG_DEBUG
  // WARNING: Do not use c++ ostream in the constructor as some of its internal
  // objects are not initialized yet. Use fprintf instead.
  debug("Initializing rng\n");
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
  assert(rng == nullptr);
#endif
  rng = std::make_unique<RNG>(seed, tid);
  assert(rng != nullptr);
#if PRISM_RNG_DEBUG
  debug("rng allocated at %p\n", (void *)rng.get());
#endif
}

auto get_rng() -> internal::RNG * {
  if (HWY_UNLIKELY(rng == nullptr)) {
    init_rng();
  }
  assert(rng != nullptr);
  return rng.get();
}

}; // namespace internal

/* API */

HWY_FLATTEN auto uniform(float f) -> internal::VF32 {
  return internal::get_rng()->Uniform(f);
}

HWY_FLATTEN auto uniform(double d) -> internal::VF64 {
  return internal::get_rng()->Uniform(d);
}

HWY_FLATTEN auto random(std::uint32_t u) -> internal::VU32 {
  return internal::get_rng()->operator()(u);
}

HWY_FLATTEN auto random(std::uint64_t u) -> internal::VU64 {
  return internal::get_rng()->operator()(u);
}

HWY_FLATTEN auto randombit(std::uint32_t u) -> internal::VU32 {
  constexpr auto u32_tag = hn::DFromV<internal::VU32>();
  static thread_local hn::Vec<decltype(u32_tag)> rand_vec;
  static thread_local size_t call_count = 0;

  if (call_count == hn::Lanes(u32_tag)) {
    rand_vec = random(u);
    call_count = 0;
  }
  return hn::Set(u32_tag, hn::ExtractLane(rand_vec, call_count++) & 1);
}

HWY_FLATTEN auto randombit(std::uint64_t u) -> internal::VU64 {
  constexpr auto u64_tag = hn::DFromV<internal::VU64>();
  static thread_local hn::Vec<decltype(u64_tag)> rand_vec;
  static thread_local size_t call_count = 0;

  if (call_count == hn::Lanes(u64_tag)) {
    rand_vec = random(u);
    call_count = 0;
  }
  return hn::Set(u64_tag, hn::ExtractLane(rand_vec, call_count++) & 1);
}

} // namespace prism::vector::xoshiro::HWY_NAMESPACE

HWY_AFTER_NAMESPACE();

#if HWY_ONCE

void debug(const char *fmt, ...) {
#if PRISM_RNG_DEBUG
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "[PRISM Debug Xoshiro] ");
  vfprintf(stderr, fmt, args);
  va_end(args);
#endif
}

__attribute__((constructor)) void init() {
  hwy::GetChosenTarget().Update(hwy::SupportedTargets());
#if PRISM_RNG_DEBUG
  debug("Target chosen: %s\n", hwy::TargetName(HWY_TARGET));
#endif
}
#endif // HWY_ONCE
