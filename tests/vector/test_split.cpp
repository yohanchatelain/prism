#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_split.cpp"
#include "hwy/foreach_target.h"
// clang-format on

#include <gtest/gtest.h>

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector-inl.h"

#include "tests/helper/common.h"
#include "tests/helper/distance.h"

#include "tests/helper/operator-inl.h"
#include "tests/helper/tests-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper;
namespace helper_simd = prism::tests::helper::HWY_NAMESPACE;
namespace test = prism::tests::helper::generic::HWY_NAMESPACE;

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void is_close(D d, const helper::ConfigTest & /*unused*/, std::tuple<V> &&arg) {

  using H = typename helper::IEEE754<T>::H;

  const auto [va] = arg;
  const auto a = helper_simd::extract_unique_lane(d, va);
  auto vx_hi = hn::Undefined(d);
  auto vx_lo = hn::Undefined(d);

  sr::Split(d, va, vx_hi, vx_lo);

  const auto N = hn::Lanes(d);

  for (auto i = 0; i < N; i++) {
    H ref = hn::ExtractLane(va, i);
    H x_hi = hn::ExtractLane(vx_hi, i);
    H x_lo = hn::ExtractLane(vx_lo, i);
    H target = x_hi + x_lo;

    if (helper::isnan(target) and helper::isnan(ref)) {
      return;
    }

    if (helper::isinf(target) and helper::isinf(ref)) {
      return;
    }

    auto abs_diff = helper::absolute_distance(ref - target);
    auto abs_rel = helper::relative_distance(ref, target);
    constexpr auto half_ulp = .5 * helper::IEEE754<T>::ulp;

    auto correct = abs_rel <= half_ulp;

    if (not correct) {
      std::cerr << std::hexfloat << "--- Failed for ---\n"
                << "a        : " << a << "\n"
                << "reference: " << helper::hexfloat(ref) << "\n"
                << "target   : " << helper::hexfloat(target) << "\n"
                << "abs_diff : " << helper::hexfloat(abs_diff) << "\n"
                << "rel_diff : " << helper::hexfloat(abs_rel) << std::endl;
    }

    // Dekker split is not valid when (C.x) overflows
    if (helper::isnan(target)) {
      return;
    }

    HWY_ASSERT(correct); // NOLINT
  }
}

constexpr auto arity = 1;
struct TestSplitBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestSimpleCase<arity>(is_close<D>, d);
  }
};

struct TestSplitRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandom01<arity>(is_close<D>, d);
  }
};

struct TestSplitBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestAllBinades<arity>(is_close<D>, d);
  }
};

HWY_NOINLINE void TestAllSplitBasicAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestSplitBasicAssertions>());
}

HWY_NOINLINE void TestAllSplitRandom01Assertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestSplitRandom01Assertions>());
}

HWY_NOINLINE void TestAllSplitBinadeAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestSplitBinadeAssertions>());
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
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitBinadeAssertions);
// NOLINTEND
HWY_AFTER_TEST();

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE