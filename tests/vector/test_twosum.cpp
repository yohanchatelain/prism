#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_twosum.cpp"
#include "hwy/foreach_target.h"
// clang-format on

#include <gtest/gtest.h>

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector-inl.h"

#include "tests/helper/common.h"
#include "tests/helper/operator.h"

#include "tests/helper/tests-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper;
namespace helper_simd = prism::tests::helper::HWY_NAMESPACE;
namespace test = prism::tests::helper::generic::HWY_NAMESPACE;

namespace reference {

// twosum reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename helper::IEEE754<T>::H>
auto twosum(T a, T b) -> H {
  return helper::reference::add(helper::Args<T>{a, b});
}
}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename H = typename helper::IEEE754<T>::H>
void is_close(D d, const helper::ConfigTest & /*unused*/,
              std::tuple<V, V> &&args) {

  const auto [va, vb] = args;
  const auto a = helper_simd::extract_unique_lane(d, va);
  const auto b = helper_simd::extract_unique_lane(d, vb);

  H ref = reference::twosum(a, b);
  T ref_cast = static_cast<T>(ref);

  V vsigma = hn::Undefined(d);
  V vtau = hn::Undefined(d);
  sr::twosum(d, va, vb, vsigma, vtau);

  const auto N = hn::Lanes(d);

  for (auto i = 0; i < N; i++) {
    H sigma = hn::ExtractLane(vsigma, i);
    H tau = hn::ExtractLane(vtau, i);
    H target = sigma + tau;

    if (helper::isnan(sigma) and helper::isnan(ref_cast)) {
      return;
    }

    if (helper::isinf(sigma) and helper::isinf(ref_cast)) {
      return;
    }

    auto abs_diff = helper::absolute_distance(ref - target);
    auto abs_rel = helper::relative_distance(ref, target);
    constexpr auto half_ulp = .5 * helper::IEEE754<T>::ulp;

    auto correct = abs_rel <= half_ulp;

    if (not correct) {
      std::cerr << std::hexfloat << "Failed for\n"
                << "a        : " << a << "\n"
                << "b        : " << b << "\n"
                << "sigma    : " << helper::hexfloat(sigma) << "\n"
                << "tau      : " << helper::hexfloat(tau) << "\n"
                << "reference: " << helper::hexfloat(ref) << "\n"
                << "target   : " << helper::hexfloat(target) << "\n"
                << "abs_diff : " << helper::hexfloat(abs_diff) << "\n"
                << "rel_diff : " << helper::hexfloat(abs_rel);
    }
    HWY_ASSERT(correct); // NOLINT
  }
}

constexpr auto arity = 2;
struct TestTwoSumBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestSimpleCase<arity>(is_close<D>, d);
  }
};

struct TestTwoSumRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandom01<arity>(is_close<D>, d);
  }
};

struct TestTwoSumRandomNoOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomNoOverlap<arity>(is_close<D>, d);
  }
};

struct TestTwoSumRandomLastBitOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomLastBitOverlap<arity>(is_close<D>, d);
  }
};

struct TestTwoSumRandomMidOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomMidOverlap<arity>(is_close<D>, d);
  }
};

struct TestTwoSumBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestAllBinades<arity>(is_close<D>, d);
  }
};

HWY_NOINLINE void TestAllTwoSumBasicAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestTwoSumBasicAssertions>());
}

HWY_NOINLINE void TestAllTwoSumRandom01Assertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestTwoSumRandom01Assertions>());
}

HWY_NOINLINE void TestAllTwoSumRandomNoOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestTwoSumRandomNoOverlapAssertions>());
}

HWY_NOINLINE void TestAllTwoSumRandomLastBitOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestTwoSumRandomLastBitOverlapAssertions>());
}

HWY_NOINLINE void TestAllTwoSumRandomMidOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestTwoSumRandomMidOverlapAssertions>());
}

HWY_NOINLINE void TestAllTwoSumBinadeAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestTwoSumBinadeAssertions>());
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
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomNoOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomLastBitOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomMidOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumBinadeAssertions);
HWY_AFTER_TEST();
// NOLINTEND
} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE