#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <gtest/gtest.h>

#include "src/utils.h"

namespace reference {
// return pred(|s|)

// compute 2 ** n
template <typename T> T pow2(int n) { return std::ldexp(1.0, n); }
}; // namespace reference

template <typename T> constexpr void test_equality(int a) {
  EXPECT_EQ(reference::pow2<T>(a), prism::utils::pow2<T>(a))
      << std::hexfloat << "Failed for\n"
      << "input    : " << a << "\n"
      << "reference: " << reference::pow2<T>(a) << "\n"
      << "target   : " << prism::utils::pow2<T>(a);
}

TEST(GetPow2Test, FullRangeAssertions) {
  auto start = prism::utils::IEEE754<float>::min_exponent_subnormal;
  auto end = prism::utils::IEEE754<float>::max_exponent;
  for (int i = start - 1; i <= end; i++) {
    test_equality<float>(i);
  }

  start = prism::utils::IEEE754<double>::min_exponent_subnormal;
  end = prism::utils::IEEE754<double>::max_exponent;
  for (int i = start - 1; i <= end; i++) {
    test_equality<double>(i);
  }
}
