// Stands for a library built by verificarlo-c with static dispatch: its
// floating-point operations call the kernels of libprism-static.so.

#include <cstdint>

#include "src/sr_scalar.h"
#include "src/utils.h"
#include "src/xoshiro.h"

namespace srs = prism::sr::scalar::static_dispatch;

extern "C" double prism_test_sum(int n) {
  double s = 0;
  for (int i = 0; i < n; i++) {
    s = srs::addf64(s, 0.1);
  }
  return s;
}

// The configuration as the static kernels see it.
extern "C" int32_t prism_test_rounding_mode() {
  return prism::sr::get_rounding_mode();
}

extern "C" uint64_t prism_test_seed() { return get_user_seed(); }
