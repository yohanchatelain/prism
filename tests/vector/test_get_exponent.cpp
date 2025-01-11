#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_get_exponent.cpp"
#include "hwy/foreach_target.h"
// clang-format on

#include <gtest/gtest.h>

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector-inl.h"

#include "tests/helper/common.h"

#include "tests/helper/operator-inl.h"
#include "tests/helper/tests-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper;
namespace helper_simd = prism::tests::helper::HWY_NAMESPACE;
namespace test = prism::tests::helper::generic::HWY_NAMESPACE;

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

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void test_equality(D d, const helper::ConfigTest & /*unused*/,
                   std::tuple<V> &&arg) {

  using DI = hn::RebindToSigned<D>;
  const DI di{};

  const auto [va] = arg;
  const auto a = helper_simd::extract_unique_lane(d, va);
  const auto reference = reference::get_exponent(a);
  const auto target = sr::get_exponent(d, va);
  const auto target_scalar = helper_simd::extract_unique_lane(di, target);

  std::hexfloat(std::cerr);

  if (target_scalar != reference) {
    std::cerr << "Failed for\n"
              << "input    : " << a << "\n"
              << "reference: " << reference << "\n"
              << "target   : " << target_scalar << "\n";
    HWY_ASSERT(false); // NOLINT
  }
}

constexpr auto arity = 1;

struct TestGetExponentBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestSimpleCase<arity>(test_equality<D>, d);
  }
};

struct TestGetExponentRandomAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandom01<arity>(test_equality<D>, d);
  }
};

struct TestGetExponentBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestAllBinades<arity>(test_equality<D>, d);
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

namespace sr::vector {
namespace {
// NOLINTBEGIN
HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentRandomAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllGetExponentBinadeAssertions);
HWY_AFTER_TEST();
// NOLINTEND
} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE