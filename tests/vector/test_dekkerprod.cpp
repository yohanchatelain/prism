#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_dekkerprod.cpp"
#include "hwy/foreach_target.h"
// clang-format on

#include <gtest/gtest.h>

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"

#include "src/debug_vector-inl.h"
#include "src/sr_vector-inl.h"

#include "tests/helper/tests.h"

#include "tests/helper/operator.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;
namespace helper_hwy = prism::tests::helper::HWY_NAMESPACE;
namespace helper = prism::tests::helper;

namespace reference {

// DekkerProd reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename R = typename helper::IEEE754<T>::H>
auto dekkerprod(T a, T b) -> R {
  std::vector<T> args = {a, b};
  return helper::reference::mul(args);
}
}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

/*
"Handbook of Floating-Point Arithmetic" by Jean-Michel Muller et al.
Theorem 4.8: Assume the minimal exponent emin and the precision p satisfy p ≥ 3
and β emin − p + 1 ≤ 1. Let ex and ey be the exponents of the floating-point
numbers x and y. If β = 2 or p is even, and if there is no overflow in the
splitting of x and y or in the computation of r1 = RN(x * y) and RN(xh * yh),
then the floating-point numbers r1 and r2 returned by Algorithm 4.10 satisfy:
1. If ex + ey ≥ emin + p − 1, then xy = r1 + r2 exactly;
2. In any case, |xy − (r1 + r2)| ≤ 7/2 . β^(emin − p + 1).
*/
template <typename T, typename H>
bool is_correct(const H x, const H y, const H r1, const H r2) {
  constexpr auto emin_p_1 = helper::IEEE754<T>::min_exponent_subnormal;
  constexpr auto beta_emin_p_1 = helper::IEEE754<T>::min_subnormal;
  constexpr auto ulp = helper::IEEE754<T>::ulp;

  const auto ex = helper::get_exponent(x);
  const auto ey = helper::get_exponent(y);

  const auto abs_error = helper::absolute_distance((x * y) - (r1 + r2));
  const auto rel_error = helper::relative_distance(x * y, r1 + r2);
  const auto case1 = (ex + ey) >= emin_p_1;
  const auto case2 = abs_error <= (3.5 * beta_emin_p_1);

  return case1 ? (rel_error < ulp) or (abs_error < 2 * beta_emin_p_1) : case2;
}

template <typename T1, typename T2> bool is_not_finite(T1 a, T2 b) {
  if (helper::isnan(a) and helper::isnan(b)) {
    return true;
  }

  if (helper::isinf(a) and helper::isinf(b)) {
    return true;
  }

  return false;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename H = typename helper::IEEE754<T>::H>
void is_close(D d, T a, T b) {

  H ref = reference::dekkerprod(a, b);
  T ref_cast = static_cast<T>(ref);
  const auto va = hn::Set(d, a);
  const auto vb = hn::Set(d, b);
  auto vpi_hi = hn::Undefined(d);
  auto vpi_lo = hn::Undefined(d);

  sr::DekkerProd(d, va, vb, vpi_hi, vpi_lo);

  H ah = static_cast<H>(helper_hwy::extract_unique_lane(d, va));
  H bh = static_cast<H>(helper_hwy::extract_unique_lane(d, vb));
  H pi_hi = static_cast<H>(helper_hwy::extract_unique_lane(d, vpi_hi));
  H pi_lo = static_cast<H>(helper_hwy::extract_unique_lane(d, vpi_lo));
  H target = pi_hi + pi_lo;

  if (is_not_finite(target, ref_cast)) {
    return;
  }

  auto correct = is_correct<T>(ah, bh, pi_hi, pi_lo);

  if (not correct) {
    const auto nametype = std::is_same_v<T, float> ? "float" : "double";
    const auto abs_diff = helper::absolute_distance(ref - target);
    const auto abs_rel = helper::relative_distance(ref, target);
    std::cerr << std::hexfloat << "Failed for\n"
              << "type     : " << nametype << "\n"
              << "a        : " << a << "\n"
              << "b        : " << b << "\n"
              << "reference: " << helper::hexfloat(ref) << "\n"
              << "target   : " << helper::hexfloat(target) << "\n"
              << "abs_diff : " << helper::hexfloat(abs_diff) << "\n"
              << "rel_diff : " << helper::hexfloat(abs_rel) << std::endl;
  }
  HWY_ASSERT(correct);
}

template <class D, typename T = hn::TFromD<D>> void do_test(D d, T a, T b) {
  is_close(d, a, b);
  is_close(d, a, -b);
  is_close(d, -a, b);
  is_close(d, -a, -b);
}

template <class D, typename T = hn::TFromD<D>>
void do_test_rng(D d, const helper::Range &range1, const helper::Range &range2,
                 const int repetitions = 100) {
  helper::RNG rng{range1.start, range1.end};
  helper::RNG rng2{range2.start, range2.end};
  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    T b = rng2();
    do_test(d, a, b);
  }
}

template <class D, typename T = hn::TFromD<D>>
void do_test_binade(D d, const int n, const int repetitions = 10) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  const auto range = helper::Range{start, end};
  do_test_rng(d, range, range, repetitions);
}

struct TestDekkerProdBasicAssertions {
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

struct TestDekkerProdRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{0.0, 1.0};
    do_test_rng(d, range1, range2);
  }
};

struct TestDekkerProdRandomNoOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-25 : 0x1p-54;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestDekkerProdRandomLastBitOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-23 : 0x1p-52;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto range1 = helper::Range{1.0, 2.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestDekkerProdRandomMidOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-12 : 0x1p-26;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-13 : 0x1p-27;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestDekkerProdBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? -149 : -1074;
    constexpr auto end = std::is_same_v<T, float> ? 127 : 1023;
    for (int i = start; i < end; i++) {
      do_test_binade(d, i);
    }
  }
};

HWY_NOINLINE void TestAllDekkerProdBasicAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestDekkerProdBasicAssertions>());
}

HWY_NOINLINE void TestAllDekkerProdRandom01Assertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestDekkerProdRandom01Assertions>());
}

HWY_NOINLINE void TestAllDekkerProdRandomNoOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestDekkerProdRandomNoOverlapAssertions>());
}

HWY_NOINLINE void TestAllDekkerProdRandomLastBitOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestDekkerProdRandomLastBitOverlapAssertions>());
}

HWY_NOINLINE void TestAllDekkerProdRandomMidOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestDekkerProdRandomMidOverlapAssertions>());
}

HWY_NOINLINE void TestAllDekkerProdBinadeAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestDekkerProdBinadeAssertions>());
}

} // namespace
// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace sr::vector::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace sr::vector {
namespace {

HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdRandomNoOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdRandomLastBitOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdRandomMidOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllDekkerProdBinadeAssertions);
HWY_AFTER_TEST();

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE