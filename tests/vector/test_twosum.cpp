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

#include "tests/helper/tests.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper::HWY_NAMESPACE;

namespace reference {

// twosum reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename R = typename helper::IEEE754<T>::H>
auto twosum(T a, T b) -> R {
  using H = typename helper::IEEE754<T>::H;
  return static_cast<H>(a) + static_cast<H>(b);
}
}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename H = typename helper::IEEE754<T>::H>
void is_close(D d, T a, T b) {

  H ref = reference::twosum(a, b);
  T ref_cast = static_cast<T>(ref);

  const auto va = hn::Set(d, a);
  const auto vb = hn::Set(d, b);
  V vsigma = hn::Undefined(d);
  V vtau = hn::Undefined(d);
  sr::twosum(d, va, vb, vsigma, vtau);

  const auto N = hn::Lanes(d);

  for (auto i = 0; i < N; i++) {
    H sigma = static_cast<H>(hn::ExtractLane(vsigma, i));
    H tau = static_cast<H>(hn::ExtractLane(vtau, i));
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
                << "a    : " << a << "\n"
                << "b    : " << b << "\n"
                << "sigma: " << (double)sigma << "\n"
                << "tau  : " << (double)tau << "\n"
                << "reference: " << (double)ref << "\n"
                << "target   : " << (double)target << "\n"
                << "abs_diff     : " << (double)abs_diff << "\n"
                << "rel_diff     : " << static_cast<double>(abs_rel);
    }
    HWY_ASSERT(correct);
  }
}

template <class D, typename T = hn::TFromD<D>> void do_test(D d, T a, T b) {
  is_close(d, a, b);
  is_close(d, a, -b);
  is_close(d, -a, b);
  is_close(d, -a, -b);
}

template <class D, typename T = hn::TFromD<D>>
void do_test_rng(D d, const helper::Range &range1, const helper::Range &range2,
                 const int repetitions = 1000) {
  helper::RNG rng{range1.start, range1.end};
  helper::RNG rng2{range2.start, range2.end};
  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    T b = rng2();
    do_test(d, a, b);
  }
}

template <class D, typename T = hn::TFromD<D>>
void do_test_binade(D d, const int n, const int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  const auto range = helper::Range{start, end};
  do_test_rng(d, range, range, repetitions);
}

struct TestTwoSumBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    auto simple_case = helper::SimpleCase<T>();
    for (const auto &a : simple_case) {
      for (const auto &b : simple_case) {
        do_test(d, a, b);
      }
    }
  }
};

struct TestTwoSumRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{0.0, 1.0};
    do_test_rng(d, range1, range2);
  }
};

struct TestTwoSumRandomNoOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-25 : 0x1p-54;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestTwoSumRandomLastBitOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-23 : 0x1p-52;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto range1 = helper::Range{1.0, 2.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestTwoSumRandomMidOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-12 : 0x1p-26;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-13 : 0x1p-27;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestTwoSumBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? -149 : -1074;
    constexpr auto end = std::is_same_v<T, float> ? 127 : 1023;
    for (int i = start; i < end; i++) {
      do_test_binade(d, i);
    }
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

HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomNoOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomLastBitOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumRandomMidOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllTwoSumBinadeAssertions);
HWY_AFTER_TEST();

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE