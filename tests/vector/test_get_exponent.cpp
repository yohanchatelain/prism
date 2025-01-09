#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_get_exponent.cpp"
#include "hwy/foreach_target.h"
// clang-format on

#include <gtest/gtest.h>

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector-inl.h"

#include "tests/helper/random.h"
#include "tests/helper/tests.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace helper = prism::tests::helper::HWY_NAMESPACE;
namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;
namespace hn = hwy::HWY_NAMESPACE;

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

namespace {

template <typename T, typename D> void test_equality(D d, T a) {
  using DI = hn::RebindToSigned<D>;
  const DI di{};

  const auto va = hn::Set(d, a);
  const auto reference = reference::get_exponent(a);
  const auto target = sr::get_exponent(d, va);

  auto target_min = hn::ReduceMin(di, target);
  auto target_max = hn::ReduceMax(di, target);

  std::hexfloat(std::cerr);

  if (target_min != target_max) {
    std::cerr << "All lane's value should be the same\n"
              << "input    : " << a << "\n"
              << "reference: " << reference << "\n"
              << "target   : " << target_min << " " << target_max << "\n";
    HWY_ASSERT(false);
  }

  if (target_min != reference) {
    std::cerr << "Failed for\n"
              << "input    : " << a << "\n"
              << "reference: " << reference << "\n"
              << "target   : " << target_min << "\n";
    HWY_ASSERT(false);
  }
}

template <typename T, class D>
void testBinade(D d, int n, int repetitions = 10) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  helper::RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    test_equality<T>(d, a);
    test_equality<T>(d, -a);
  }
}

struct TestGetExponentBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    auto simple_case = helper::get_simple_case<T>();
    for (auto a : simple_case) {
      test_equality<T>(d, a);
      test_equality<T>(d, -a);
    }
  }
};

struct TestGetExponentRandomAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    helper::RNG rng;
    for (int i = 0; i < 1000; i++) {
      T a = rng();
      test_equality<T>(d, a);
      test_equality<T>(d, -a);
    }
  }
};

struct TestGetExponentBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? -149 : -1074;
    constexpr auto end = std::is_same_v<T, float> ? 127 : 1023;
    for (int i = start; i < end; i++) {
      testBinade<T>(d, i);
    }
  }
};

HWY_NOINLINE void TestAllGetExponentBasicAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestGetExponentBasicAssertions>());
}

HWY_NOINLINE void TestAllGetExponentRandomAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestGetExponentRandomAssertions>());
}

HWY_NOINLINE void TestAllGetExponentBinadeAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestGetExponentBinadeAssertions>());
}

} // namespace
// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace sr::vector::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace sr {
namespace vector {
namespace {

HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentRandomAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentBinadeAssertions);
HWY_AFTER_TEST();

} // namespace
} // namespace vector
} // namespace sr

HWY_TEST_MAIN();

#endif // HWY_ONCE