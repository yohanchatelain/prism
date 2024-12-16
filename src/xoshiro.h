#ifndef __PRISM_XOSHIRO_H__
#define __PRISM_XOSHIRO_H__

#include <iostream>
#include <random>

#include "hwy/base.h"

namespace prism {

HWY_API uint64_t get_user_seed() throw() {
  static bool initialized = false;
  static uint64_t seed = 0;

  if (initialized) {
    return seed;
  }

  // try to get PRISM_RNG_SEED from the environment
  const char *env_seed = getenv("PRISM_RNG_SEED");
  if (env_seed) {
    try {
      seed = std::stoll(env_seed, nullptr, 10);
    } catch (const std::exception &e) {
      std::cerr << "Error parsing VFC_RNG_SEED: ";
      std::cerr << e.what() << '\n';
      std::cerr << "setting seed to random\n";
    }
    initialized = true;
  }

  if (not initialized) {
    std::random_device rd;
    seed = rd();
    initialized = true;
  }

  return seed;
}

namespace scalar {
namespace xoshiro {

namespace static_dispatch {

HWY_DLLEXPORT float uniform(float);
HWY_DLLEXPORT double uniform(double);
HWY_DLLEXPORT std::uint64_t random();

} // namespace static_dispatch

namespace dynamic_dispatch {

HWY_DLLEXPORT float uniform(float);
HWY_DLLEXPORT double uniform(double);
HWY_DLLEXPORT std::uint64_t random();

} // namespace dynamic_dispatch

} // namespace xoshiro
} // namespace scalar

} // namespace prism

#endif // __PRISM_XOSHIRO_H__