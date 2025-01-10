#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <gtest/gtest.h>

#include "src/utils.h"
#include "tests/helper/tests.h"

namespace helper = prism::tests::helper;

namespace reference {
template <typename T> auto get_exponent(T a) -> int32_t {
  int exp = 0;
  auto cls = std::fpclassify(a);
  switch (cls) {
  case FP_ZERO:
    return 0;
  case FP_NORMAL:
    std::frexp(a, &exp);
    return exp - 1;
    break;
  case FP_SUBNORMAL:
    return std::numeric_limits<T>::min_exponent - 2;
  case FP_INFINITE:
  case FP_NAN:
    return std::numeric_limits<T>::max_exponent;
  default:
    std::cerr << "Error: unknown classification\n";
    std::abort();
    break;
  }
}
}; // namespace reference

template <typename T> void test_equality(T a) {
  const auto ref = reference::get_exponent(a);
  const auto target = prism::utils::get_exponent(a);
  EXPECT_EQ(ref, target) << std::hexfloat << "Failed for\n"
                         << "input    : " << a << "\n"
                         << "reference: " << ref << "\n"
                         << "target   : " << target;
}

constexpr auto arity = 1;

TEST(GetExponentTest, BasicAssertions) {
  helper::TestBasic<float, arity>(test_equality<float>);
  helper::TestBasic<double, arity>(test_equality<double>);
}

TEST(GetExponentTest, RandomAssertions) {
  helper::TestRandom01<float, arity>(test_equality<float>);
  helper::TestRandom01<double, arity>(test_equality<double>);
}

TEST(GetExponentTest, BinadeAssertions) {
  helper::TestAllBinades<float, arity>(test_equality<float>);
  helper::TestAllBinades<double, arity>(test_equality<double>);
}