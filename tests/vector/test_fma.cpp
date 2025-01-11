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

#include "tests/helper/distance.h"
#include "tests/helper/operator.h"

#include "tests/helper/operator-inl.h"
#include "tests/helper/tests-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper;
namespace helper_simd = prism::tests::helper::HWY_NAMESPACE;
namespace test = prism::tests::helper::generic::HWY_NAMESPACE;

namespace reference {

// Fma reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename helper::IEEE754<T>::H>
auto fma(T a, T b, T c) -> H {
  std::vector<T> args = {a, b, c};
  return helper::reference::fma(args);
}
}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
auto will_overflow(const D d, const V a, const V b, const V c) -> bool {
  const auto prod = hn::Mul(a, b);
  const auto ref = hn::Add(prod, c);
  bool overflow = false;
  overflow |= hn::AllFalse(d, hn::IsFinite(prod));
  overflow |= hn::AllFalse(d, hn::IsFinite(ref));
  return overflow;
}

template <typename T, typename H>
auto is_correct(const H a, const H b, const H c, const H result) -> bool {
  constexpr auto ulp = helper::IEEE754<T>::ulp;
  constexpr auto beta_emin_p_1 = helper::IEEE754<T>::min_subnormal;

  const auto reference = reference::fma(a, b, c);

  const auto abs_error = helper::absolute_distance(reference - result);
  const auto rel_error = helper::relative_distance(reference, result);

  const auto case1 = abs_error < 2 * beta_emin_p_1;
  const auto case2 = rel_error < ulp;

  return case1 or case2;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void is_close(D d, T a, T b, T c) {

  using H = typename helper::IEEE754<T>::H;

  const auto ref = reference::fma(a, b, c);
  const auto ref_cast = static_cast<T>(ref);

  const auto va = hn::Set(d, a);
  const auto vb = hn::Set(d, b);
  const auto vc = hn::Set(d, c);

  const auto vres = sr::fma_emul(d, va, vb, vc);

  if (will_overflow(d, va, vb, vc)) {
    return;
  }

  H ah = helper_simd::extract_unique_lane(d, va);
  H bh = helper_simd::extract_unique_lane(d, vb);
  H ch = helper_simd::extract_unique_lane(d, vc);
  H target = helper_simd::extract_unique_lane(d, vres);

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
  HWY_ASSERT(correct); // NOLINT
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void do_test(D d, const helper::ConfigTest & /*unused*/,
             std::tuple<V, V, V> &&args) {
  const auto [va, vb, vc] = args;
  const auto a = helper_simd::extract_unique_lane(d, va);
  const auto b = helper_simd::extract_unique_lane(d, vb);
  const auto c = helper_simd::extract_unique_lane(d, vc);
  is_close(d, a, b, c);
}

constexpr auto arity = 3;

struct TestFmaBasicAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestSimpleCase<arity>(do_test<D>, d);
  }
};

struct TestFmaRandom01Assertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandom01<arity>(do_test<D>, d);
  }
};

struct TestFmaRandomNoOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomNoOverlap<arity>(do_test<D>, d);
  }
};

struct TestFmaRandomLastBitOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomLastBitOverlap<arity>(do_test<D>, d);
  }
};

struct TestFmaRandomMidOverlapAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestRandomMidOverlap<arity>(do_test<D>, d);
  }
};

struct TestFmaBinadeAssertions {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestAllBinades<arity>(do_test<D>, d);
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
// NOLINTBEGIN
HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaBasicAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandom01Assertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomNoOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomLastBitOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaRandomMidOverlapAssertions);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllFmaBinadeAssertions);
HWY_AFTER_TEST();
// NOLINTEND

} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE