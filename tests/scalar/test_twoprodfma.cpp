#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <gtest/gtest.h>

#include "src/eft.h"
#include "src/utils.h"

#include "tests/helper/tests.h"

#include "tests/helper/distance.h"

namespace helper = prism::tests::helper;

namespace reference {
// return pred(|s|)

// twoprodfma reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename helper::IEEE754<T>::H>
auto twoprodfma(T a, T b) -> H {
  return helper::reference::mul(helper::Args<T>{a, b});
}

}; // namespace reference

template <typename T> void is_close(T a, T b) {
  if (std::isnan(a) or std::isnan(b) or std::isinf(a) or std::isinf(b)) {
    return;
  }

  const auto ref = reference::twoprodfma(a, b);
  const auto ref_cast = static_cast<T>(ref);
  T x = 0;
  T e = 0;
  twoprodfma(a, b, x, e);
  const auto target = helper::reference::add(helper::Args<T>{x, e});

  if (std::isnan(x) and std::isnan(ref_cast)) {
    return;
  }

  if (std::isinf(x) and std::isinf(ref_cast)) {
    return;
  }

  auto diff = helper::absolute_distance(ref, target);
  auto rel = helper::relative_distance(ref, target);

  auto ulp = helper::IEEE754<T>::ulp;
  auto min_subnormal = helper::IEEE754<T>::min_subnormal;
  auto min_normal = helper::IEEE754<T>::min_normal;

  T error_bound = 0;
  bool correct = false;
  if (diff == 0) {
    correct = true;
  } else if (diff < min_subnormal) {
    correct = true;
  } else if (diff < min_normal) {
    error_bound = ulp;
    correct = rel <= error_bound;
  } else {
    error_bound = .5 * ulp;
    correct = rel <= error_bound;
  }

  EXPECT_TRUE(correct) << std::hexfloat << "Failed for\n"
                       << "type     : " << typeid(a).name() << "\n"
                       << "ulp      : " << ulp << "\n"
                       << "bound    : " << error_bound << "\n"
                       << "lowest   : " << min_subnormal << "\n"
                       << "a        : " << a << "\n"
                       << "b        : " << b << "\n"
                       << "target   : " << "(" << x << " , " << e << ")\n"
                       << "reference: " << helper::hexfloat(ref) << "\n"
                       << "target   : " << helper::hexfloat(target) << "\n"
                       << "abs_diff : " << helper::hexfloat(diff) << "\n"
                       << "rel_diff : " << helper::hexfloat(rel);
}

template <typename T> void test_equality(T a, T b) { is_close(a, b); }

constexpr auto arity = 2;

TEST(GetTwoProdFmaTest, BasicAssertions) {
  helper::TestBasic<float, arity>(test_equality<float>);
  helper::TestBasic<double, arity>(test_equality<double>);
}

TEST(GetTwoProdFmaTest, Random01Assertions) {
  helper::TestRandom01<float, arity>(test_equality<float>);
  helper::TestRandom01<double, arity>(test_equality<double>);
}

TEST(GetTwoProdFmaTest, RandomNoOverlapAssertions) {
  helper::TestRandomNoOverlap<float, arity>(test_equality<float>);
  helper::TestRandomNoOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoProdFmaTest, RandomLastBitOverlapAssertions) {
  helper::TestRandomLastBitOverlap<float, arity>(test_equality<float>);
  helper::TestRandomLastBitOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoProdFmaTest, RandomMidOverlapAssertions) {
  helper::TestRandomMidOverlap<float, arity>(test_equality<float>);
  helper::TestRandomMidOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoProdFmaTest, BinadeAssertions) {
  helper::TestAllBinades<float, arity>(test_equality<float>);
  helper::TestAllBinades<double, arity>(test_equality<double>);
}