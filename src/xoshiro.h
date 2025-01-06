#ifndef __PRISM_XOSHIRO_H__
#define __PRISM_XOSHIRO_H__

#include <random>

__attribute__((unused)) static int get_user_seed() {
  static bool initialized = false;
  static int seed = 0;
  if (initialized == false) {
    const char *HWY_RESTRICT seed_env_str = "PRISM_SEED";
    const char *HWY_RESTRICT seed_str = getenv(seed_env_str);
    if (seed_str != nullptr) {
      seed = atoll(seed_str);
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

#include "hwy/base.h"
#include "src/random-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism::scalar::xoshiro {
namespace HWY_NAMESPACE {

float uniform(float);
double uniform(double);
std::uint64_t random();

} // namespace HWY_NAMESPACE
} // namespace prism::scalar::xoshiro

namespace prism::vector::xoshiro {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

hn::Vec<hn::ScalableTag<float>> uniform(float);
hn::Vec<hn::ScalableTag<double>> uniform(double);
hn::Vec<hn::ScalableTag<std::uint32_t>> random(std::uint32_t);
hn::Vec<hn::ScalableTag<std::uint64_t>> random(std::uint64_t);

} // namespace HWY_NAMESPACE
} // namespace prism::vector::xoshiro
HWY_AFTER_NAMESPACE(); // at file scope

#endif // PRISM_XOSHIRO_H_
