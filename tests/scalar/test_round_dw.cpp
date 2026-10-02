// Tests for rounding a double-word number (x, e), i.e. the unevaluated sum
// x + e with x = RN(x + e), through interflop_prism_round_dw_binary{32,64}.
//
// Built twice, against prism-dynamic and prism-static; PRISM_DISPATCH selects
// the matching C++ namespace.

#include <cfloat>
#include <cmath>
#include <limits>
#include <random>
#include <type_traits>

#include <gtest/gtest.h>

#include "src/eft.h"
#include "src/prism_api.h"
#include "src/sr_scalar.h"
#include "src/utils.h"
#include "tests/helper/binomial_test.h"

namespace sr = prism::sr::scalar::PRISM_DISPATCH;
namespace helper = prism::tests::helper;

namespace {

constexpr int kRepetitions = 10'000;
constexpr double kAlpha = 0.00001;

template <typename T> auto round_dw(T x, T e) -> T {
  if constexpr (std::is_same_v<T, float>) {
    return interflop_prism_round_dw_binary32(x, e);
  } else {
    return interflop_prism_round_dw_binary64(x, e);
  }
}

template <typename T> auto add(T a, T b) -> T {
  if constexpr (std::is_same_v<T, float>) {
    return sr::addf32(a, b);
  } else {
    return sr::addf64(a, b);
  }
}

template <typename T> void set_precision(int32_t t) {
  if constexpr (std::is_same_v<T, float>) {
    interflop_prism_set_thread_virtual_precision_binary32(t);
  } else {
    interflop_prism_set_thread_virtual_precision_binary64(t);
  }
}

// Every result must be one of the two neighbours of x + e, and `target` must
// be hit with probability p.
template <typename T>
void expect_probability(T x, T e, T target, T other, double p) {
  int hits = 0;
  for (int i = 0; i < kRepetitions; i++) {
    const T r = round_dw(x, e);
    ASSERT_TRUE(r == target or r == other)
        << std::hexfloat << "round_dw(" << x << ", " << e << ") = " << r;
    hits += (r == target) ? 1 : 0;
  }
  const auto test = helper::binomial_test(kRepetitions, hits, p);
  EXPECT_GT(test.pvalue, kAlpha)
      << std::hexfloat << "round_dw(" << x << ", " << e << "): " << hits
      << "/" << kRepetitions << " hits on " << target << ", expected p = " << p;
}

class RoundDWTest : public ::testing::Test {
protected:
  void SetUp() override { reset(); }
  void TearDown() override { reset(); }

  static void reset() {
    interflop_prism_set_default_virtual_precision_binary32(24);
    interflop_prism_set_default_virtual_precision_binary64(53);
    interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
  }
};

template <typename T> void check_special_values() {
  constexpr T inf = std::numeric_limits<T>::infinity();
  constexpr T nan = std::numeric_limits<T>::quiet_NaN();

  for (const int32_t mode : {INTERFLOP_PRISM_SR, INTERFLOP_PRISM_RN}) {
    interflop_prism_set_rounding_mode(mode);
    EXPECT_TRUE(std::isnan(round_dw(nan, T{0})));
    EXPECT_TRUE(std::isnan(round_dw(T{1}, nan)));
    EXPECT_TRUE(std::isnan(round_dw(inf, -inf)));
    EXPECT_EQ(round_dw(inf, T{0}), inf);
    EXPECT_EQ(round_dw(-inf, T{0}), -inf);

    EXPECT_EQ(round_dw(T{0}, T{0}), T{0});
    EXPECT_FALSE(std::signbit(round_dw(T{0}, T{0})));
    EXPECT_EQ(round_dw(-T{0}, -T{0}), T{0});
    EXPECT_TRUE(std::signbit(round_dw(-T{0}, -T{0})));
  }
}

TEST_F(RoundDWTest, SpecialValuesBinary32) { check_special_values<float>(); }
TEST_F(RoundDWTest, SpecialValuesBinary64) { check_special_values<double>(); }

// Overflow follows the arithmetic operations: rounding past the largest
// finite value gives infinity.
TEST_F(RoundDWTest, Overflow) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);
  set_precision<float>(10);
  set_precision<double>(10);
  EXPECT_EQ(round_dw(FLT_MAX, 0.0F), std::numeric_limits<float>::infinity());
  EXPECT_EQ(round_dw(DBL_MAX, 0.0), std::numeric_limits<double>::infinity());
  EXPECT_EQ(round_dw(-DBL_MAX, 0.0), -std::numeric_limits<double>::infinity());
}

// Values representable at the virtual precision are returned unchanged.
template <typename T> void check_exact_values() {
  set_precision<T>(10);
  const T ulp_t = std::ldexp(T{1}, -9);
  for (const int32_t mode : {INTERFLOP_PRISM_SR, INTERFLOP_PRISM_RN}) {
    interflop_prism_set_rounding_mode(mode);
    for (int i = 0; i < 100; i++) {
      EXPECT_EQ(round_dw(T{1}, T{0}), T{1});
      EXPECT_EQ(round_dw(T{1} + ulp_t, T{0}), T{1} + ulp_t);
      EXPECT_EQ(round_dw(-T{3} * ulp_t, T{0}), -T{3} * ulp_t);
    }
  }
}

TEST_F(RoundDWTest, ExactValuesBinary32) { check_exact_values<float>(); }
TEST_F(RoundDWTest, ExactValuesBinary64) { check_exact_values<double>(); }

// At full precision, x is already RN(x + e): only the low part e tells SR
// how far x + e lies from x.
template <typename T> void check_sr_full_precision() {
  constexpr int p = std::numeric_limits<T>::digits;
  const T ulp = std::ldexp(T{1}, 1 - p);  // ulp(1)
  const T quarter = ulp / 4;              // a quarter ulp above 1
  const T below = std::ldexp(T{1}, -p);   // ulp below 1

  // 1 + ulp/4 rounds up with probability 1/4.
  expect_probability<T>(T{1}, quarter, T{1} + ulp, T{1}, 0.25);
  expect_probability<T>(-T{1}, -quarter, -T{1} - ulp, -T{1}, 0.25);

  // 1 - ulp/4 lies in the binade below 1, where the spacing is ulp/2, so it
  // is halfway between 1 - ulp/2 and 1.
  expect_probability<T>(T{1}, -quarter, T{1} - below, T{1}, 0.5);
}

TEST_F(RoundDWTest, SRUsesLowPartBinary32) {
  check_sr_full_precision<float>();
}
TEST_F(RoundDWTest, SRUsesLowPartBinary64) {
  check_sr_full_precision<double>();
}

TEST_F(RoundDWTest, SRReducedPrecision) {
  set_precision<float>(10);
  set_precision<double>(10);
  // 1 + ulp_t/4 at t = 10
  expect_probability<float>(1.0F + 0x1p-11F, 0.0F, 1.0F + 0x1p-9F, 1.0F, 0.25);
  expect_probability<double>(1.0 + 0x1p-11, 0.0, 1.0 + 0x1p-9, 1.0, 0.25);
  // 1 + 3 ulp_t/4, with a low part that does not move the probability
  // measurably
  expect_probability<double>(1.0 + 0x1.8p-10, 0x1p-60, 1.0 + 0x1p-9, 1.0,
                             0.75);
}

// At reduced precision, x may sit exactly on a tie of the virtual grid; the
// sign of e then decides the rounding direction.
template <typename T> void check_rn_reduced_precision(T tiny) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);
  set_precision<T>(10);
  const T ulp_t = std::ldexp(T{1}, -9);
  const T tie = T{1} + ulp_t / 2;

  EXPECT_EQ(round_dw(tie, tiny), T{1} + ulp_t);
  EXPECT_EQ(round_dw(tie, -tiny), T{1});
  // A true tie rounds away from zero.
  EXPECT_EQ(round_dw(tie, T{0}), T{1} + ulp_t);
  EXPECT_EQ(round_dw(-tie, tiny), -T{1});
  EXPECT_EQ(round_dw(-tie, -tiny), -T{1} - ulp_t);

  EXPECT_EQ(round_dw(T{1} + T{3} * ulp_t / 4, T{0}), T{1} + ulp_t);
  EXPECT_EQ(round_dw(T{1} + ulp_t / 4, tiny), T{1});
}

TEST_F(RoundDWTest, RNReducedPrecisionBinary32) {
  check_rn_reduced_precision<float>(0x1p-30F);
}
TEST_F(RoundDWTest, RNReducedPrecisionBinary64) {
  check_rn_reduced_precision<double>(0x1p-60);
}

// add(a, b) is twosum followed by the same rounding, so in RN mode rounding
// the twosum pair must give the same result as add.
template <typename T> void check_consistency_with_add(int32_t t) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);
  set_precision<T>(t);

  std::mt19937_64 gen(t);
  std::uniform_real_distribution<T> mantissa(-2, 2);
  std::uniform_int_distribution<int> exponent(-30, 30);

  for (int i = 0; i < 10'000; i++) {
    const T a = std::ldexp(mantissa(gen), exponent(gen));
    const T b = std::ldexp(mantissa(gen), exponent(gen));
    T s;
    T e;
    twosum(a, b, s, e);
    ASSERT_EQ(round_dw(s, e), add(a, b))
        << std::hexfloat << "t = " << t << ", a = " << a << ", b = " << b;
  }
}

TEST_F(RoundDWTest, ConsistentWithAddBinary32) {
  for (const int32_t t : {4, 10, 16, 24}) {
    check_consistency_with_add<float>(t);
  }
}

TEST_F(RoundDWTest, ConsistentWithAddBinary64) {
  for (const int32_t t : {4, 10, 24, 40, 53}) {
    check_consistency_with_add<double>(t);
  }
}

// The C API and the C++ dispatch namespace give the same result.
TEST_F(RoundDWTest, CppAPI) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);
  set_precision<float>(10);
  set_precision<double>(10);
  EXPECT_EQ(sr::round_dwf32(1.0F + 0x1p-10F, -0x1p-30F), 1.0F);
  EXPECT_EQ(sr::round_dwf32(1.0F + 0x1p-10F, 0x1p-30F), 1.0F + 0x1p-9F);
  EXPECT_EQ(sr::round_dwf64(1.0 + 0x1p-10, -0x1p-60), 1.0);
  EXPECT_EQ(sr::round_dwf64(1.0 + 0x1p-10, 0x1p-60), 1.0 + 0x1p-9);
}

} // namespace
