#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <type_traits>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_fma.cpp"
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

namespace reference {

// Fma reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename R = typename helper::IEEE754<T>::H>
auto fma(T a, T b, T c) -> R {
  std::vector<T> args = {a, b, c};
  return helper::reference::fma(args);
}
}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
bool will_overflow(const D d, const V a, const V b, const V c) {
  const auto prod = hn::Mul(a, b);
  const auto ref = hn::Add(prod, c);
  bool overflow = false;
  overflow |= hn::AllFalse(d, hn::IsFinite(prod));
  overflow |= hn::AllFalse(d, hn::IsFinite(ref));
  return overflow;
}

template <typename T, typename H>
bool is_correct(const H a, const H b, const H c, const H result) {
  constexpr auto ulp = helper::IEEE754<T>::ulp;
  constexpr auto beta_emin_p_1 = helper::IEEE754<T>::min_subnormal;

  const auto reference = reference::fma(a, b, c);

  const auto abs_error = helper::absolute_distance(reference - result);
  const auto rel_error = helper::relative_distance(reference, result);

  const auto case1 = abs_error < 2 * beta_emin_p_1;
  const auto case2 = rel_error < ulp;

  return case1 or case2;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename H = typename helper::IEEE754<T>::H>
void is_close(D d, T a, T b, T c) {

  H ref = reference::fma(a, b, c);
  T ref_cast = static_cast<T>(ref);

  const auto va = hn::Set(d, a);
  const auto vb = hn::Set(d, b);
  const auto vc = hn::Set(d, c);

  const auto vres = sr::fma_emul(d, va, vb, vc);

  if (will_overflow(d, va, vb, vc)) {
    return;
  }

  H ah = static_cast<H>(helper::extract_unique_lane(d, va));
  H bh = static_cast<H>(helper::extract_unique_lane(d, vb));
  H ch = static_cast<H>(helper::extract_unique_lane(d, vc));
  H target = static_cast<H>(helper::extract_unique_lane(d, vres));

  if (helper::isnan(target) and helper::isnan(ref_cast)) {
    return;
  }

  if (helper::isinf(target) and helper::isinf(ref_cast)) {
    return;
  }

  auto correct = is_correct<T>(ah, bh, ch, target);
  
  if (not correct) {
    const auto nametype = std::is_same_v<T, float> ? "float" : "double";
    auto abs_diff = helper::absolute_distance(ref - target);
    auto abs_rel = helper::relative_distance(ref, target);

    std::cerr << std::hexfloat << "Failed for\n"
              << "type     : " << nametype << "\n"
              << "a        : " << a << "\n"
              << "b        : " << b << "\n"
              << "c        : " << c << "\n"
              << "reference: " << helper::hexfloat(ref) << "\n"
              << "target   : " << helper::hexfloat(target) << "\n"
              << "abs_diff : " << helper::hexfloat(abs_diff) << "\n"
              << "rel_diff : " << helper::hexfloat(abs_rel) << std::endl;
  }
  HWY_ASSERT(correct);
}

template <class D, typename T = hn::TFromD<D>>
void do_test(D d, T a, T b, T c) {
  is_close(d, a, b, c);
  is_close(d, a, -b, c);
  is_close(d, -a, b, c);
  is_close(d, -a, -b, c);
  is_close(d, a, b, -c);
  is_close(d, a, -b, -c);
  is_close(d, -a, b, -c);
  is_close(d, -a, -b, -c);
}

template <class D, typename T = hn::TFromD<D>>
void do_test_rng(D d, const helper::Range &range1, const helper::Range &range2,
                 const int repetitions = 1000) {
  helper::RNG rng{range1.start, range1.end};
  helper::RNG rng2{range2.start, range2.end};
  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    T b = rng2();
    T c = rng2();
    do_test(d, a, b, c);
  }
}

template <class D, typename T = hn::TFromD<D>>
void do_test_binade(D d, const int n, const int repetitions = 10) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  const auto range = helper::Range{start, end};
  do_test_rng(d, range, range, repetitions);
}

struct TestFmaBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    auto simple_case = helper::get_simple_case<T>();
    for (const auto &a : simple_case) {
      for (const auto &b : simple_case) {
        for (const auto &c : simple_case) {
          do_test(d, a, b, c);
        }
      }
    }
  }
};

struct TestFmaRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{0.0, 1.0};
    do_test_rng(d, range1, range2);
  }
};

struct TestFmaRandomNoOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-25 : 0x1p-54;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestFmaRandomLastBitOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-23 : 0x1p-52;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-24 : 0x1p-53;
    constexpr auto range1 = helper::Range{1.0, 2.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestFmaRandomMidOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? 0x1p-12 : 0x1p-26;
    constexpr auto end = std::is_same_v<T, float> ? 0x1p-13 : 0x1p-27;
    constexpr auto range1 = helper::Range{0.0, 1.0};
    constexpr auto range2 = helper::Range{start, end};
    do_test_rng(d, range1, range2);
  }
};

struct TestFmaBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    constexpr auto start = std::is_same_v<T, float> ? -149 : -1074;
    constexpr auto end = std::is_same_v<T, float> ? 127 : 1023;
    for (int i = start; i < end; i++) {
      do_test_binade(d, i);
    }
  }
};

HWY_NOINLINE void TestAllFmaBasicAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestFmaBasicAssertions>());
}

HWY_NOINLINE void TestAllFmaRandom01Assertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestFmaRandom01Assertions>());
}

HWY_NOINLINE void TestAllFmaRandomNoOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestFmaRandomNoOverlapAssertions>());
}

HWY_NOINLINE void TestAllFmaRandomLastBitOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestFmaRandomLastBitOverlapAssertions>());
}

HWY_NOINLINE void TestAllFmaRandomMidOverlapAssertions() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestFmaRandomMidOverlapAssertions>());
}

HWY_NOINLINE void TestAllFmaBinadeAssertions() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestFmaBinadeAssertions>());
}

} // namespace
// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace sr::vector::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace sr::vector {
namespace {

HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomNoOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomLastBitOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomMidOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaBinadeAssertions);
HWY_AFTER_TEST();

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE