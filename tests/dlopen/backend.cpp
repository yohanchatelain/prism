// Stands for libinterflop_prism.so: configures PRISM through
// libprism-dynamic.so.

#include <cstdint>

#include "src/prism_api.h"

extern "C" void prism_test_configure(int32_t mode, uint64_t seed) {
  interflop_prism_set_rounding_mode(mode);
  interflop_prism_set_seed(seed);
}
