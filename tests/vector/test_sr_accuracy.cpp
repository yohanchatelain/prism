#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_sr_accuracy.cpp"
#include "hwy/foreach_target.h"

#include "hwy/highway.h"
#include "hwy/base.h"
#include "hwy/tests/test_util-inl.h"

#include "src/debug_vector-inl.h"
#include "src/sr_vector-inl.h"

#include "tests/helper/assert.h"
#include "tests/helper/tests.h"

// clang-format on

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper::HWY_NAMESPACE;

using M = helper::RoundingMode::SR;
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

namespace ud = prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE;

struct SRAdd : public helper::PrAdd {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::add(d, a, b);
  }
};

struct SRSub : public helper::PrSub {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::sub(d, a, b);
  }
};

struct SRMul : public helper::PrMul {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::mul(d, a, b);
  }
};

struct SRDiv : public helper::PrDiv {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a, V b) const -> V {
    return ud::div(d, a, b);
  }
};

struct SRSqrt : public helper::PrSqrt {
  template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
  auto operator()(D d, V a) const -> V {
    return ud::sqrt(d, a);
  }
};

struct SRFma : public helper::PrFma {
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
    helper::TestExactAdd<M, SRAdd>(d, config);
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
    helper::TestSimpleCase<M, Op>(d, config);
  }
};

using TestBasicAssertionsAdd = TestBasicAssertions<SRAdd>;
using TestBasicAssertionsSub = TestBasicAssertions<SRSub>;
using TestBasicAssertionsMul = TestBasicAssertions<SRMul>;
using TestBasicAssertionsDiv = TestBasicAssertions<SRDiv>;
using TestBasicAssertionsSqrt = TestBasicAssertions<SRSqrt>;
using TestBasicAssertionsFma = TestBasicAssertions<SRFma>;

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
    helper::TestRandom<M, Op>(d, config);
  }
};

using TestRandom01AssertionsAdd = TestRandom01Assertions<SRAdd>;
using TestRandom01AssertionsSub = TestRandom01Assertions<SRSub>;
using TestRandom01AssertionsMul = TestRandom01Assertions<SRMul>;
using TestRandom01AssertionsDiv = TestRandom01Assertions<SRDiv>;
using TestRandom01AssertionsSqrt = TestRandom01Assertions<SRSqrt>;
using TestRandom01AssertionsFma = TestRandom01Assertions<SRFma>;

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
    helper::TestRandomNoOverlap<M, Op>(d, config);
  }
};

using TestRandomNoOverlapAssertionsAdd = TestRandomNoOverlapAssertions<SRAdd>;
using TestRandomNoOverlapAssertionsSub = TestRandomNoOverlapAssertions<SRSub>;
using TestRandomNoOverlapAssertionsMul = TestRandomNoOverlapAssertions<SRMul>;
using TestRandomNoOverlapAssertionsDiv = TestRandomNoOverlapAssertions<SRDiv>;
using TestRandomNoOverlapAssertionsSqrt = TestRandomNoOverlapAssertions<SRSqrt>;
using TestRandomNoOverlapAssertionsFma = TestRandomNoOverlapAssertions<SRFma>;

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
    helper::TestRandomLastBitOverlap<M, Op>(d, config);
  }
};

using TestRandomLastBitOverlapAdd = TestRandomLastBitOverlap<SRAdd>;
using TestRandomLastBitOverlapSub = TestRandomLastBitOverlap<SRSub>;
using TestRandomLastBitOverlapMul = TestRandomLastBitOverlap<SRMul>;
using TestRandomLastBitOverlapDiv = TestRandomLastBitOverlap<SRDiv>;
using TestRandomLastBitOverlapSqrt = TestRandomLastBitOverlap<SRSqrt>;
using TestRandomLastBitOverlapFma = TestRandomLastBitOverlap<SRFma>;

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
    helper::TestRandomMidOverlap<M, Op>(d, config);
  }
};

using TestRandomMidOverlapAdd = TestRandomMidOverlap<SRAdd>;
using TestRandomMidOverlapSub = TestRandomMidOverlap<SRSub>;
using TestRandomMidOverlapMul = TestRandomMidOverlap<SRMul>;
using TestRandomMidOverlapDiv = TestRandomMidOverlap<SRDiv>;
using TestRandomMidOverlapSqrt = TestRandomMidOverlap<SRSqrt>;
using TestRandomMidOverlapFma = TestRandomMidOverlap<SRFma>;

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
HWY_BEFORE_TEST(SRVectorAccuracyTest);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllExactOperationsAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsSub);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsMul);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsDiv);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsSqrt);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllBasicAssertionsFma);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsSub);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsMul);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsDiv);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsSqrt);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandom01AssertionsFma);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsSub);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsMul);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsDiv);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsSqrt);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest,
                      TestAllRandomNoOverlapAssertionsFma);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapSub);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapMul);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapDiv);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapSqrt);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomLastBitOverlapFma);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapAdd);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapSub);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapMul);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapDiv);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapSqrt);
HWY_EXPORT_AND_TEST_P(SRVectorAccuracyTest, TestAllRandomMidOverlapFma);
HWY_AFTER_TEST();
// NOLINTEND

} // namespace prism::HWY_NAMESPACE

HWY_TEST_MAIN();

#endif // HWY_ONCE
