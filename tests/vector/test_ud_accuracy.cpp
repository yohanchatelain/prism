#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_ud_accuracy.cpp"
#include "hwy/foreach_target.h"

#include "hwy/highway.h"
#include "hwy/base.h"
#include "hwy/tests/test_util-inl.h"

#include "src/debug_vector-inl.h"
#include "src/ud_vector-inl.h"

#include "tests/helper/tests-inl.h"

// clang-format on

HWY_BEFORE_NAMESPACE(); // NOLINT

namespace prism::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace test_distribution = prism::tests::helper::distribution::HWY_NAMESPACE;
namespace helper = prism::tests::helper;

using M = helper::RoundingMode::UD;
namespace {

constexpr auto default_repetitions_values = 10'000;
constexpr auto default_alpha = 0.000001;

auto get_repetitions() -> int {
  static std::mutex mtx;
  std::lock_guard<std::mutex> lock(mtx);
  const char *env_repetitions = getenv("PRISM_TEST_REPETITIONS");
  if (env_repetitions != nullptr) {
    return std::stoi(env_repetitions);
  }
#ifdef SR_DEBUG
  return 100;
#else
  return default_repetitions_values;
#endif
}

auto get_alpha() -> double {
  static std::mutex mtx;
  std::lock_guard<std::mutex> lock(mtx);
  const char *env_alpha = getenv("PRISM_TEST_THRESHOLD");
  if (env_alpha != nullptr) {
    return std::stod(env_alpha);
  }
  return default_alpha;
}

auto default_repetitions() -> int {
  static const int repetitions = get_repetitions();
  return repetitions;
}

namespace ud = prism::ud::vector::PRISM_DISPATCH::HWY_NAMESPACE;

struct UDAdd : public helper::PrAdd {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::add(d, a, b);
  }
};

struct UDSub : public helper::PrSub {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::sub(d, a, b);
  }
};

struct UDMul : public helper::PrMul {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::mul(d, a, b);
  }
};

struct UDDiv : public helper::PrDiv {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::div(d, a, b);
  }
};

struct UDSqrt : public helper::PrSqrt {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a) const -> V {
    return ud::sqrt(d, a);
  }
};

struct UDFma : public helper::PrFma {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b, V c) const -> V {
    return ud::fma(d, a, b, c);
  }
};

struct TestExactOperationsAdd {
  helper::ConfigTest config = {.name = "TestExactOperationsAdd",
                               .description =
                                   "Test exact operations for addition",
                               .repetitions = default_repetitions(),
                               .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestExactAdd<M, UDAdd>(d, config);
  }
};

HWY_NOINLINE void TestAllExactOperationsAdd() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestExactOperationsAdd>());
}

template <class Op> struct TestBasicAssertions {
  helper::ConfigTest config = {
      .name = "TestBasicAssertions",
      .description = "Test basic assertions for arithmetic operations",
      .repetitions = default_repetitions(),
      .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestSimpleCase<M, Op>(d, config);
  }
};

using TestBasicAssertionsAdd = TestBasicAssertions<UDAdd>;
using TestBasicAssertionsSub = TestBasicAssertions<UDSub>;
using TestBasicAssertionsMul = TestBasicAssertions<UDMul>;
using TestBasicAssertionsDiv = TestBasicAssertions<UDDiv>;
using TestBasicAssertionsSqrt = TestBasicAssertions<UDSqrt>;
using TestBasicAssertionsFma = TestBasicAssertions<UDFma>;

HWY_NOINLINE void TestAllBasicAssertionsAdd() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsAdd>());
}

HWY_NOINLINE void TestAllBasicAssertionsSub() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsSub>());
}

HWY_NOINLINE void TestAllBasicAssertionsMul() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsMul>());
}

HWY_NOINLINE void TestAllBasicAssertionsDiv() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsDiv>());
}

HWY_NOINLINE void TestAllBasicAssertionsSqrt() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsSqrt>());
}

HWY_NOINLINE void TestAllBasicAssertionsFma() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestBasicAssertionsFma>());
}

template <class Op> struct TestRandom01Assertions {
  helper::ConfigTest config = {
      .name = "TestRandom01Assertions",
      .description =
          "Test random numbers in (0,1) assertions for arithmetic operations",
      .repetitions = default_repetitions(),
      .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestRandom01<M, Op>(d, config);
  }
};

using TestRandom01AssertionsAdd = TestRandom01Assertions<UDAdd>;
using TestRandom01AssertionsSub = TestRandom01Assertions<UDSub>;
using TestRandom01AssertionsMul = TestRandom01Assertions<UDMul>;
using TestRandom01AssertionsDiv = TestRandom01Assertions<UDDiv>;
using TestRandom01AssertionsSqrt = TestRandom01Assertions<UDSqrt>;
using TestRandom01AssertionsFma = TestRandom01Assertions<UDFma>;

HWY_NOINLINE void TestAllRandom01AssertionsAdd() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsAdd>());
}

HWY_NOINLINE void TestAllRandom01AssertionsSub() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsSub>());
}

HWY_NOINLINE void TestAllRandom01AssertionsMul() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsMul>());
}

HWY_NOINLINE void TestAllRandom01AssertionsDiv() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsDiv>());
}

HWY_NOINLINE void TestAllRandom01AssertionsSqrt() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsSqrt>());
}

HWY_NOINLINE void TestAllRandom01AssertionsFma() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandom01AssertionsFma>());
}

template <class Op> struct TestRandomNoOverlapAssertions {
  helper::ConfigTest config = {
      .name = "TestRandomNoOverlapAssertions",
      .description = "Test random with no overlap assertions for arithmetic ",
      .repetitions = default_repetitions(),
      .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestRandomNoOverlap<M, Op>(d, config);
  }
};

using TestRandomNoOverlapAssertionsAdd = TestRandomNoOverlapAssertions<UDAdd>;
using TestRandomNoOverlapAssertionsSub = TestRandomNoOverlapAssertions<UDSub>;
using TestRandomNoOverlapAssertionsMul = TestRandomNoOverlapAssertions<UDMul>;
using TestRandomNoOverlapAssertionsDiv = TestRandomNoOverlapAssertions<UDDiv>;
using TestRandomNoOverlapAssertionsSqrt = TestRandomNoOverlapAssertions<UDSqrt>;
using TestRandomNoOverlapAssertionsFma = TestRandomNoOverlapAssertions<UDFma>;

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsAdd() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsAdd>());
}

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsSub() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsSub>());
}

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsMul() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsMul>());
}

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsDiv() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsDiv>());
}

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsSqrt() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsSqrt>());
}

HWY_NOINLINE void TestAllRandomNoOverlapAssertionsFma() {
  hn::ForFloat3264Types(
      hn::ForPartialVectors<TestRandomNoOverlapAssertionsFma>());
}

template <class Op> struct TestRandomLastBitOverlap {
  helper::ConfigTest config = {
      .name = "TestRandomLastBitOverlap",
      .description =
          "Test random with last bit overlapping assertions for arithmetic ",
      .repetitions = default_repetitions(),
      .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestRandomLastBitOverlap<M, Op>(d, config);
  }
};

using TestRandomLastBitOverlapAdd = TestRandomLastBitOverlap<UDAdd>;
using TestRandomLastBitOverlapSub = TestRandomLastBitOverlap<UDSub>;
using TestRandomLastBitOverlapMul = TestRandomLastBitOverlap<UDMul>;
using TestRandomLastBitOverlapDiv = TestRandomLastBitOverlap<UDDiv>;
using TestRandomLastBitOverlapSqrt = TestRandomLastBitOverlap<UDSqrt>;
using TestRandomLastBitOverlapFma = TestRandomLastBitOverlap<UDFma>;

HWY_NOINLINE void TestAllRandomLastBitOverlapAdd() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapAdd>());
}

HWY_NOINLINE void TestAllRandomLastBitOverlapSub() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapSub>());
}

HWY_NOINLINE void TestAllRandomLastBitOverlapMul() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapMul>());
}

HWY_NOINLINE void TestAllRandomLastBitOverlapDiv() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapDiv>());
}

HWY_NOINLINE void TestAllRandomLastBitOverlapSqrt() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapSqrt>());
}

HWY_NOINLINE void TestAllRandomLastBitOverlapFma() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomLastBitOverlapFma>());
}

template <class Op> struct TestRandomMidOverlap {
  helper::ConfigTest config = {
      .name = "TestRandomMidOverlap",
      .description =
          "Test random number with mid overlap assertions for arithmetic",
      .repetitions = default_repetitions(),
      .alpha = get_alpha()};

  template <typename T, class D> void operator()(T /*unused*/, D d) {
    test_distribution::TestRandomMidOverlap<M, Op>(d, config);
  }
};

using TestRandomMidOverlapAdd = TestRandomMidOverlap<UDAdd>;
using TestRandomMidOverlapSub = TestRandomMidOverlap<UDSub>;
using TestRandomMidOverlapMul = TestRandomMidOverlap<UDMul>;
using TestRandomMidOverlapDiv = TestRandomMidOverlap<UDDiv>;
using TestRandomMidOverlapSqrt = TestRandomMidOverlap<UDSqrt>;
using TestRandomMidOverlapFma = TestRandomMidOverlap<UDFma>;

HWY_NOINLINE void TestAllRandomMidOverlapAdd() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapAdd>());
}

HWY_NOINLINE void TestAllRandomMidOverlapSub() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapSub>());
}

HWY_NOINLINE void TestAllRandomMidOverlapMul() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapMul>());
}

HWY_NOINLINE void TestAllRandomMidOverlapDiv() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapDiv>());
}

HWY_NOINLINE void TestAllRandomMidOverlapSqrt() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapSqrt>());
}

HWY_NOINLINE void TestAllRandomMidOverlapFma() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestRandomMidOverlapFma>());
}

} // namespace
} // namespace prism::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace prism::HWY_NAMESPACE {
// NOLINTBEGIN
HWY_BEFORE_TEST(UDVectorAccuracyTest);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllExactOperationsAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsSub);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsMul);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsDiv);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsSqrt);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllBasicAssertionsFma);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsSub);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsMul);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsDiv);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsSqrt);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandom01AssertionsFma);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsSub);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsMul);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsDiv);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsSqrt);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsFma);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapSub);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapMul);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapDiv);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapSqrt);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomLastBitOverlapFma);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapAdd);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapSub);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapMul);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapDiv);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapSqrt);
HWY_EXPORT_AND_TEST_P(UDVectorAccuracyTest, TestAllRandomMidOverlapFma);
HWY_AFTER_TEST();
// NOLINTEND
} // namespace prism::HWY_NAMESPACE

HWY_TEST_MAIN();

#endif // HWY_ONCE
