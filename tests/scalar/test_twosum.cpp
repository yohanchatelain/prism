#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <gtest/gtest.h>

#include "src/eft.h"
#include "src/utils.h"

#include "tests/helper/tests.h"

#include "tests/helper/distance.h"
#include "tests/helper/operator.h"
#include "tests/helper/random.h"

namespace helper = prism::tests::helper::HWY_NAMESPACE;

namespace reference {
// return pred(|s|)

// twosum reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename R = typename helper::IEEE754<T>::H>
auto twosum(T a, T b) -> R {
  using H = typename helper::IEEE754<T>::H;
  return static_cast<H>(a) + static_cast<H>(b);
}

}; // namespace reference

template <typename T, typename H = typename helper::IEEE754<T>::H>
void is_close(T a, T b) {
  if (std::isnan(a) or std::isnan(b) or std::isinf(a) or std::isinf(b)) {
    return;
  }

  H ref = reference::twosum(a, b);
  T ref_cast = static_cast<T>(ref);
  T x = 0;
  T e = 0;
  twosum(a, b, x, e);
  H target = x + e;

  if (std::isnan(x) and std::isnan(ref_cast)) {
    return;
  }

  if (std::isinf(x) and std::isinf(ref_cast)) {
    return;
  }

  auto diff = helper::absolute_distance(ref - target);
  auto rel = helper::relative_distance(ref, target);
  constexpr auto half_ulp = .5 * helper::IEEE754<T>::ulp;

  auto correct = rel <= half_ulp;

  EXPECT_TRUE(correct) << std::hexfloat << "Failed for\n"
                       << "a    : " << a << "\n"
                       << "b    : " << b << "\n"
                       << "reference: " << (double)ref << "\n"
                       << "target   : " << (double)target << "\n"
                       << "abs_diff     : " << (double)diff << "\n"
                       << "rel_diff     : " << static_cast<double>(rel);
}

template <typename T> struct Pair {
  T x;
  T e;

  friend auto operator<<(std::ostream &os, const Pair<T> &p) -> std::ostream & {
    os << "(" << p.x << ", " << p.e << ")";
    return os;
  }
};

template <typename T> constexpr void test_equality(T a, T b) { is_close(a, b); }

template <typename T> void testBinade(int n, int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  helper::RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    T b = rng();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }
}

TEST(GetTwoSumTest, BasicAssertions) {

  const auto simple_case_float = helper::get_simple_case<float>();
  for (auto a : simple_case_float) {
    for (auto b : simple_case_float) {
      test_equality(a, b);
      test_equality(a, -b);
      test_equality(-a, b);
      test_equality(-a, -b);
    }
  }

  const auto simple_case_double = helper::get_simple_case<double>();
  for (auto a : simple_case_double) {
    for (auto b : simple_case_double) {
      test_equality(a, b);
      test_equality(a, -b);
      test_equality(-a, b);
      test_equality(-a, -b);
    }
  }
}

TEST(GetTwoSumTest, Random01Assertions) {
  helper::RNG rng;

  for (int i = 0; i < 1000; i++) {
    float a = rng();
    float b = rng();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }

  for (int i = 0; i < 1000; i++) {
    double a = rng();
    double b = rng();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }
}

TEST(GetTwoSumTest, RandomNoOverlapAssertions) {
  helper::RNG rng_0v1_float(0, 1);
  helper::RNG rng_24v25_float(0x1p-24, 0x1p-25);

  for (int i = 0; i < 1000; i++) {
    float a = rng_0v1_float();
    float b = rng_24v25_float();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }

  helper::RNG rng_0v1_double(0, 1);
  helper::RNG rng_53v54_double(0x1p-53, 0x1p-54);

  for (int i = 0; i < 1000; i++) {
    double a = rng_0v1_double();
    double b = rng_53v54_double();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }
}

TEST(GetTwoSumTest, RandomLastBitOverlapAssertions) {
  helper::RNG rng_1v2_float(1, 2);
  helper::RNG rng_23v24_float(0x1p-23, 0x1p-24);

  for (int i = 0; i < 1000; i++) {
    float a = rng_1v2_float();
    float b = rng_23v24_float();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }

  helper::RNG rng_1v2_double(1, 2);
  helper::RNG rng_52v53_double(0x1p-52, 0x1p-53);

  for (int i = 0; i < 1000; i++) {
    double a = rng_1v2_double();
    double b = rng_52v53_double();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }
}

TEST(GetTwoSumTest, RandomMidOverlapAssertions) {
  helper::RNG rng_0v1_float(0, 1);
  helper::RNG rng_12v13_float(0x1p-12, 0x1p-13);

  for (int i = 0; i < 1000; i++) {
    float a = rng_0v1_float();
    float b = rng_12v13_float();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }

  helper::RNG rng_0v1_double(0, 1);
  helper::RNG rng_26v27_double(0x1p-26, 0x1p-27);

  for (int i = 0; i < 1000; i++) {
    double a = rng_0v1_double();
    double b = rng_26v27_double();
    test_equality(a, b);
    test_equality(a, -b);
    test_equality(-a, b);
    test_equality(-a, -b);
  }
}

TEST(GetTwoSumTest, BinadeAssertions) {
  for (int i = -126; i < 127; i++) {
    testBinade<float>(i);
  }
  for (int i = -1022; i < 1023; i++) {
    testBinade<double>(i);
  }
}