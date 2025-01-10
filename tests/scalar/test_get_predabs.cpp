#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <gtest/gtest.h>

#include "src/utils.h"
#include "tests/helper/random.h"
#include "tests/helper/tests.h"

namespace helper_hwy = prism::tests::helper::HWY_NAMESPACE;
namespace helper = prism::tests::helper;

namespace reference {
// return pred(|s|)

// ulp = SIGN(t) * ldexp(1, get_predecessor_abs(s*(1-ldexp(1, -53))) - 52);
// pred|s| = s * (1 - ldexp(1, -53))
// eta = get_predecessor_abs(pred(|s|))
// ulp = sign(tau) * 2^eta * eps
template <typename T> auto get_predecessor_abs(T a) -> T {
  constexpr uint32_t p = prism::utils::IEEE754<T>::precision;
  return a * (1 - std::ldexp(1.0, -p));
}
}; // namespace reference

template <typename T> auto is_nan_or_inf(T a) -> bool {
  const auto ref = reference::get_predecessor_abs(a);
  const auto target = prism::utils::get_predecessor_abs(a);
  if (std::isnan(a)) {
    EXPECT_TRUE(std::isnan(ref));
    EXPECT_TRUE(std::isnan(target));
    return true;
  }
  if (std::isinf(a)) {
    EXPECT_TRUE(std::isinf(ref));
    EXPECT_TRUE(std::isinf(target));
    return true;
  }
  return false;
}

template <typename T> void test_equality(T a) {
  if (not is_nan_or_inf(a)) {
    const auto ref = reference::get_predecessor_abs(a);
    const auto target = prism::utils::get_predecessor_abs(a);
    EXPECT_EQ(ref, target) << std::hexfloat << "Failed for\n"
                           << "input    : " << a << "\n"
                           << "reference: " << ref << "\n"
                           << "target   : " << target;
  }
}

constexpr auto arity = 1;

TEST(GetPredAbsTest, BasicAssertions) {
  helper::TestBasic<float, arity>(test_equality<float>);
  helper::TestBasic<double, arity>(test_equality<double>);
}

TEST(GetPredAbsTest, RandomAssertions) {
  helper::TestRandom01<float, arity>(test_equality<float>);
  helper::TestRandom01<double, arity>(test_equality<double>);
}

TEST(GetPredAbsTest, BinadeAssertions) {
  helper::TestAllBinades<float, arity>(test_equality<float>);
  helper::TestAllBinades<double, arity>(test_equality<double>);
}