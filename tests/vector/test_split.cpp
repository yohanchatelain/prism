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

#include "tests/helper/tests.h"

#include "tests/helper/operator.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper::HWY_NAMESPACE;

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename H = typename helper::IEEE754<T>::H>
void is_close(D d, T a) {

  const auto va = hn::Set(d, a);
  auto vx_hi = hn::Undefined(d);
  auto vx_lo = hn::Undefined(d);

  sr::Split(d, va, vx_hi, vx_lo);

  const auto N = hn::Lanes(d);

  for (auto i = 0; i < N; i++) {
    H ref = static_cast<H>(hn::ExtractLane(va, i));
    H x_hi = static_cast<H>(hn::ExtractLane(vx_hi, i));
    H x_lo = static_cast<H>(hn::ExtractLane(vx_lo, i));
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
                << "reference: " << (double)ref << "\n"
                << "target   : " << (double)target << "\n"
                << "abs_diff : " << (double)abs_diff << "\n"
                << "rel_diff : " << static_cast<double>(abs_rel) << std::endl;
    }

    // Dekker split is not valid when (C.x) overflows
    if (helper::isnan(target)) {
      return;
    }

    HWY_ASSERT(correct);
  }
}

template <class D, typename T = hn::TFromD<D>> void do_test(D d, T a) {
  is_close(d, a);
  is_close(d, -a);
}

template <class D, typename T = hn::TFromD<D>>
void do_test_rng(D d, const helper::Range &range1,
                 const int repetitions = 100) {
  helper::RNG rng{range1.start, range1.end};
  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    do_test(d, a);
  }
}

template <class D, typename T = hn::TFromD<D>>
void do_test_binade(D d, const int n, const int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  const auto range = helper::Range{start, end};
  do_test_rng(d, range, repetitions);
}

struct TestSplitBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    auto simple_case = helper::get_simple_case<T>();
    for (const auto &a : simple_case) {
      do_test(d, a);
    }
  }
};

struct TestSplitRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto range = helper::Range{0.0, 1.0};
    do_test_rng(d, range);
  }
};

struct TestSplitBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? -149 : -1074;
    constexpr auto end = std::is_same_v<T, float> ? 127 : 1023;
    for (int i = start; i < end; i++) {
      do_test_binade(d, i);
    }
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

HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllSplitBinadeAssertions);
HWY_AFTER_TEST();

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE