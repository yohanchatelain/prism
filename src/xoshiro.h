#ifndef __PRISM_XOSHIRO_H__
#define __PRISM_XOSHIRO_H__

#include <random>

__attribute__((unused)) static auto get_user_seed() -> uint32_t {
  static bool initialized = false;
  static uint32_t seed = 0;
  if (not initialized) {
    const char *seed_env_str = "PRISM_SEED";
    const char *seed_str = getenv(seed_env_str);
    if (seed_str != nullptr) {
      char *endptr = nullptr;
      seed = strtoll(seed_str, &endptr, 10);
      if (*endptr != '\0') {
        // Handle conversion error
        seed = 0; // or some default value
      }
    } else {
      std::random_device rd;
      seed = rd();
    }
  }
  return seed;
}

#endif // __PRISM_XOSHIRO_H__

#if defined(PRISM_XOSHIRO_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_XOSHIRO_H_
#undef PRISM_XOSHIRO_H_
#else
#define PRISM_XOSHIRO_H_
#endif

#include "src/random-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism::scalar::xoshiro::HWY_NAMESPACE {

auto uniform(float) -> float;
auto uniform(double) -> double;
auto random() -> std::uint64_t;

} // namespace prism::scalar::xoshiro::HWY_NAMESPACE

namespace prism::vector::xoshiro::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

auto uniform(float) -> hn::Vec<hn::ScalableTag<float>>;
auto uniform(double) -> hn::Vec<hn::ScalableTag<double>>;
auto random(std::uint32_t) -> hn::Vec<hn::ScalableTag<std::uint32_t>>;
auto random(std::uint64_t) -> hn::Vec<hn::ScalableTag<std::uint64_t>>;

} // namespace prism::vector::xoshiro::HWY_NAMESPACE
HWY_AFTER_NAMESPACE(); // at file scope

#endif // PRISM_XOSHIRO_H_
