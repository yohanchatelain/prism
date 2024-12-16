#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "src/utils.h"
#include "tests/helper.h"

namespace reference {
template <typename T> int32_t get_exponent(T a) {
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

struct RNG {

  std::random_device rd;

private:
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

public:
  RNG(double a = 0.0, double b = 1.0) : gen(RNG::rd()), dis(a, b){};
  double operator()() { return dis(gen); }
};

template <typename T> std::vector<T> get_simple_case() {
  std::vector<T> simple_case = {0.0,
                                1.0,
                                2.0,
                                3.0,
                                std::numeric_limits<T>::min(),
                                std::numeric_limits<T>::lowest(),
                                std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::epsilon(),
                                std::numeric_limits<T>::infinity(),
                                std::numeric_limits<T>::denorm_min(),
                                std::numeric_limits<T>::quiet_NaN(),
                                std::numeric_limits<T>::signaling_NaN()};
  return simple_case;
}

#define test_equality(a)                                                       \
  EXPECT_EQ(reference::get_exponent(a), prism::utils::get_exponent(a))         \
      << std::hexfloat << "Failed for\n"                                       \
      << "input    : " << a << "\n"                                            \
      << "reference: " << reference::get_exponent(a) << "\n"                   \
      << "target   : " << prism::utils::get_exponent(a);

template <typename T> void testBinade(int n, int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    test_equality(a);
    test_equality(-a);
  }
}

TEST(GetExponentTest, BasicAssertions) {
  std::vector<float> simple_case_float = get_simple_case<float>();
  for (auto a : simple_case_float) {
    test_equality(a);
    test_equality(-a);
  }

  std::vector<double> simple_case_double = get_simple_case<double>();
  for (auto a : simple_case_double) {
    test_equality(a);
    test_equality(-a);
  }
}

TEST(GetExponentTest, RandomAssertions) {
  RNG rng;

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