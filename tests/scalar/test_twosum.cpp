#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <gtest/gtest.h>

#include "src/eft.h"

#include "tests/helper/distance.h"
#include "tests/helper/operator.h"
#include "tests/helper/tests.h"

namespace helper = prism::tests::helper;

namespace reference {
// return pred(|s|)

// twosum reference
// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename helper::IEEE754<T>::H>
auto twosum(T a, T b) -> H {
  return helper::reference::add(helper::Args<T>{a, b});
}

}; // namespace reference

template <typename T, typename H = typename helper::IEEE754<T>::H>
void is_close(T a, T b) {
  if (std::isnan(a) or std::isnan(b) or std::isinf(a) or std::isinf(b)) {
    return;
  }

  H ref = reference::twosum(a, b);
  T ref_cast = static_cast<T>(ref);
  T x = 0;
  T e = 0;
  twosum(a, b, x, e);
  H target = x + e;

  if (std::isnan(x) and std::isnan(ref_cast)) {
    return;
  }

  if (std::isinf(x) and std::isinf(ref_cast)) {
    return;
  }

  auto diff = helper::absolute_distance(ref - target);
  auto rel = helper::relative_distance(ref, target);
  constexpr auto half_ulp = .5 * helper::IEEE754<T>::ulp;

  auto correct = rel <= half_ulp;

  EXPECT_TRUE(correct) << std::hexfloat << "Failed for\n"
                       << "a        : " << a << "\n"
                       << "b        : " << b << "\n"
                       << "reference: " << helper::hexfloat(ref) << "\n"
                       << "target   : " << helper::hexfloat(target) << "\n"
                       << "abs_diff : " << helper::hexfloat(diff) << "\n"
                       << "rel_diff : " << helper::hexfloat(rel);
}

template <typename T> struct Pair {
  T x;
  T e;

  friend auto operator<<(std::ostream &os, const Pair<T> &p) -> std::ostream & {
    os << "(" << p.x << ", " << p.e << ")";
    return os;
  }
};

template <typename T> void test_equality(T a, T b) { is_close(a, b); }

constexpr auto arity = 2;

TEST(GetTwoSumTest, BasicAssertions) {
  helper::TestBasic<float, arity>(test_equality<float>);
  helper::TestBasic<double, arity>(test_equality<double>);
}

TEST(GetTwoSumTest, Random01Assertions) {
  helper::TestRandom01<float, arity>(test_equality<float>);
  helper::TestRandom01<double, arity>(test_equality<double>);
}

TEST(GetTwoSumTest, RandomNoOverlapAssertions) {
  helper::TestRandomNoOverlap<float, arity>(test_equality<float>);
  helper::TestRandomNoOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoSumTest, RandomLastBitOverlapAssertions) {
  helper::TestRandomLastBitOverlap<float, arity>(test_equality<float>);
  helper::TestRandomLastBitOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoSumTest, RandomMidOverlapAssertions) {
  helper::TestRandomMidOverlap<float, arity>(test_equality<float>);
  helper::TestRandomMidOverlap<double, arity>(test_equality<double>);
}

TEST(GetTwoSumTest, BinadeAssertions) {
  helper::TestAllBinades<float, arity>(test_equality<float>);
  helper::TestAllBinades<double, arity>(test_equality<double>);
}