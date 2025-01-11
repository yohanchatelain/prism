#include "tests/helper/operator.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_get_pow2.cpp"
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
// return pred(|s|)

// compute 2 ** n
auto pow2(int n) -> double { return std::ldexp(1.0, n); }

}; // namespace reference

namespace {

namespace sr = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void test_equality(D d, const helper::ConfigTest & /*unused*/,
                   const std::tuple<V> &&arg) {

  using DI = hn::RebindToSigned<D>;
  const DI di{};

  const auto [va] = arg;
  const auto a = helper_simd::extract_unique_lane(d, va);
  // input is float, so we need to extract the exponent
  const auto exp = helper::get_exponent(a);
  const auto vexp = hn::Set(di, exp);

  const auto reference = reference::pow2(exp);
  const auto target = sr::pow2(d, vexp);
  const auto target_scalar = helper_simd::extract_unique_lane(d, target);

  if (target_scalar != reference) {
    std::cerr << std::hexfloat;
    std::cerr << "Failed for\n"
              << "type     : " << typeid(T).name() << "\n"
              << "input    : " << exp << "\n"
              << "reference: " << reference << "\n"
              << "target   : " << target_scalar << "\n";
    HWY_ASSERT(false); // NOLINT
  }
}

constexpr auto arity = 1;
struct TestPow2 {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /* unused */, D d) {
    test::TestAllBinades<arity>(test_equality<D>, d);
  }
};

HWY_NOINLINE void TestAllPow2() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestPow2>());
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
HWY_EXPORT_AND_TEST_P(SRTest, TestAllPow2);
HWY_AFTER_TEST();
// NOLINTEND
} // namespace
} // namespace sr::vector

HWY_TEST_MAIN();

#endif // HWY_ONCE