#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <gtest/gtest.h>

#include "src/utils.h"
#include "tests/helper/random.h"
#include "tests/helper/tests.h"

namespace helper = prism::tests::helper::HWY_NAMESPACE;

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

#define test_equality(a)                                                       \
  EXPECT_EQ(reference::get_exponent(a), prism::utils::get_exponent(a))         \
      << std::hexfloat << "Failed for\n"                                       \
      << "input    : " << a << "\n"                                            \
      << "reference: " << reference::get_exponent(a) << "\n"                   \
      << "target   : " << prism::utils::get_exponent(a);

template <typename T> void testBinade(int n, int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  helper::RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    test_equality(a);
    test_equality(-a);
  }
}

TEST(GetExponentTest, BasicAssertions) {
  const auto simple_case_float = helper::get_simple_case<float>();
  for (auto a : simple_case_float) {
    test_equality(a);
    test_equality(-a);
  }

  const auto simple_case_double = helper::get_simple_case<double>();
  for (auto a : simple_case_double) {
    test_equality(a);
    test_equality(-a);
  }
}

TEST(GetExponentTest, RandomAssertions) {
  helper::RNG rng;

  for (int i = 0; i < 1000; i++) {
    float a = rng();
    test_equality(a);
    test_equality(-a);
  }

  for (int i = 0; i < 1000; i++) {
    double a = rng();
    test_equality(a);
    test_equality(-a);
  }
}

TEST(GetExponentTest, BinadeAssertions) {
  constexpr auto start_float = -149;
  constexpr auto start_double = -1074;
  constexpr auto end_float = 127;
  constexpr auto end_double = 1023;
  for (int i = start_float; i < end_float; i++) {
    testBinade<float>(i);
  }
  for (int i = start_double; i < end_double; i++) {
    testBinade<double>(i);
  }
}