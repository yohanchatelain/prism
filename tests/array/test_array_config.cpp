#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_api.h"
#include "src/prism_array.h"

namespace {

TEST(ArrayConfigTest, SRVirtualPrecisionArray) {
  const size_t N = 64;
  std::vector<float> a(N, 1.0f);
  // b is chosen such that under full precision (24 bits) it has effect, but under 8 bits it truncates
  std::vector<float> b(N, 0x1.0p-15f);
  std::vector<float> res(N, 0.0f);

  // Set precision to 8 bits
  interflop_prism_set_default_virtual_precision_binary32(8);

  prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), N);

  // Results should either be 1.0 or 1.0 + 2^(-7) = 1.0078125
  for (size_t i = 0; i < N; ++i) {
    EXPECT_TRUE(res[i] == 1.0f || res[i] == 1.0f + 0x1.0p-7f);
  }

  // Restore precision
  interflop_prism_set_default_virtual_precision_binary32(24);
}

TEST(ArrayConfigTest, RNModeArray) {
  const size_t N = 64;
  std::vector<float> a(N, 1.0f);
  std::vector<float> b(N, 0x1.0p-25f);
  std::vector<float> res(N, 0.0f);

  // Set rounding mode to RN (deterministic round to nearest)
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), N);

  // In RN mode, 1.0 + 2^(-25) rounds deterministically to 1.0 (since 2^(-25) < 0.5 * 2^(-23))
  for (size_t i = 0; i < N; ++i) {
    EXPECT_EQ(res[i], 1.0f);
  }

  // Set rounding mode back to SR
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

} // namespace
