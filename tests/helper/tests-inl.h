#include "tests/helper/common.h"
#include <array>
#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#if defined(PRISM_TESTS_HELPER_TESTS_TOGGLE) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_TESTS_HELPER_TESTS_TOGGLE
#undef PRISM_TESTS_HELPER_TESTS_TOGGLE
#else
#define PRISM_TESTS_HELPER_TESTS_TOGGLE
#endif

#include "hwy/highway.h"

#include "src/utils.h"
#include "tests/helper/random.h"
#include "tests/helper/tests.h"

#include "tests/helper/assert-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::tests::helper::generic::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestExactAdd(TestFunc &&check, const D d, const ConfigTest &config) {

  constexpr int32_t mantissa = prism::utils::IEEE754<T>::mantissa;
  constexpr auto repetitions = 5;

  for (auto i = 0; i <= repetitions; i++) {
    auto va = hn::Set(d, 1.25);
    auto vb = hn::Set(d, std::ldexp(1.0, -(mantissa + i)));
    check(d, config, std::forward_as_tuple(va, vb));
  }
}

// TODO(yohan): Parallelize this function
template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestSimpleCase(TestFunc &&check, D d, ConfigTest config = {}) {
  const auto simple_case = SimpleCase<T>();

  if constexpr (arity == 1) {
    for (auto a : simple_case) {
      auto va = hn::Set(d, a);
      check(d, config, std::forward_as_tuple(va));
    }
  } else if constexpr (arity == 2) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        auto va = hn::Set(d, a);
        auto vb = hn::Set(d, b);
        check(d, config, std::forward_as_tuple(va, vb));
      }
    }
  } else if constexpr (arity == 3) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        for (auto c : simple_case) {
          auto va = hn::Set(d, a);
          auto vb = hn::Set(d, b);
          auto vc = hn::Set(d, c);
          check(d, config, std::forward_as_tuple(va, vb, vc));
        }
      }
    }
  }
}

template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandom(TestFunc &&check, D d, const ConfigTest &config,
                std::array<Range, arity> ranges) {

  constexpr auto repetitions = 10;

  if constexpr (arity == 1) {
    auto [range_1st] = ranges;
    RNG rng1(range_1st.start, range_1st.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      auto va = hn::Set(d, a);
      check(d, config, std::forward_as_tuple(va));
      check(d, config, std::forward_as_tuple(hn::Neg(va)));
    }
  } else if constexpr (arity == 2) {
    auto [range_1st, range_2nd] = ranges;
    RNG rng1(range_1st.start, range_1st.end);
    RNG rng2(range_2nd.start, range_2nd.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      T b = rng2();
      auto va = hn::Set(d, a);
      auto vb = hn::Set(d, b);
      check(d, config, std::forward_as_tuple(va, vb));
      check(d, config, std::forward_as_tuple(va, hn::Neg(vb)));
    }
  } else if constexpr (arity == 3) {
    auto [range_1st, range_2nd, range_3rd] = ranges;
    RNG rng1(range_1st.start, range_1st.end);
    RNG rng2(range_2nd.start, range_2nd.end);
    RNG rng3(range_3rd.start, range_3rd.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      T b = rng2();
      T c = rng3();
      auto va = hn::Set(d, a);
      auto vb = hn::Set(d, b);
      auto vc = hn::Set(d, c);
      check(d, config, std::forward_as_tuple(va, vb, vc));
      check(d, config, std::forward_as_tuple(va, hn::Neg(vb), vc));
      check(d, config, std::forward_as_tuple(va, vb, hn::Neg(vc)));
    }
  }
}

template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandom01(TestFunc &&check, D d, const ConfigTest config = {}) {
  const auto range = Range{0, 1};
  std::array<Range, arity> ranges;
  ranges.fill(range);
  TestRandom<arity>(check, d, config, ranges);
}

template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomNoOverlap(TestFunc &&check, D d, const ConfigTest config = {}) {
  constexpr auto s2 = IEEE754<T>::precision - 1;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl = take_n_first<arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, arity> ranges = tuple_to_array(ranges_tpl);
  TestRandom<arity>(check, d, config, ranges);
}

template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomLastBitOverlap(TestFunc &&check, D d,
                              const ConfigTest config = {}) {
  constexpr auto s2 = IEEE754<T>::precision;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl = take_n_first<arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, arity> ranges = tuple_to_array(ranges_tpl);
  TestRandom<arity>(check, d, config, ranges);
}

template <size_t arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomMidOverlap(TestFunc &&check, D d, const ConfigTest config = {}) {
  constexpr auto s2 = IEEE754<T>::precision / 2;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl = take_n_first<arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, arity> ranges = tuple_to_array(ranges_tpl);
  TestRandom<arity>(check, d, config, ranges);
}

template <int arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestBinade(TestFunc &&test, D d, const ConfigTest config = {}, int n = 0) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  const auto range = Range{start, end};
  const auto ranges_tpl = take_n_first<arity>(range, range, range);
  const std::array<Range, arity> ranges = tuple_to_array(ranges_tpl);
  TestRandom<arity>(test, d, config, ranges);
}

template <int arity, typename TestFunc, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestAllBinades(TestFunc &&test, D d, const ConfigTest config = {}) {
  constexpr auto min_exp = IEEE754<T>::min_exponent_subnormal;
  constexpr auto max_exp = IEEE754<T>::max_exponent;
  for (int i = min_exp; i < max_exp; i++) {
    TestBinade<arity>(test, d, config, i);
  }
}

}; // namespace prism::tests::helper::generic::HWY_NAMESPACE

namespace prism::tests::helper::distribution::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace helper = prism::tests::helper::HWY_NAMESPACE;
namespace generic = prism::tests::helper::generic::HWY_NAMESPACE;

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestExactAdd(D d, const ConfigTest &config) {

  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestExactAdd(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestSimpleCase(D d, const ConfigTest &config) {
  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestSimpleCase<Op::arity>(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandom01(D d, const ConfigTest &config) {
  const auto range = Range{0, 1};
  std::array<Range, Op::arity> ranges;
  ranges.fill(range);

  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestRandom<Op::arity>(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config,
      ranges);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomNoOverlap(D d, const ConfigTest &config) {

  constexpr auto s2 = IEEE754<T>::precision - 1;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl =
      take_n_first<Op::arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, Op::arity> ranges = tuple_to_array(ranges_tpl);
  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestRandom<Op::arity>(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config,
      ranges);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomLastBitOverlap(D d, const ConfigTest &config) {
  constexpr auto s2 = IEEE754<T>::precision;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl =
      take_n_first<Op::arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, Op::arity> ranges = tuple_to_array(ranges_tpl);
  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestRandom<Op::arity>(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config,
      ranges);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomMidOverlap(D d, const ConfigTest &config) {
  constexpr auto s2 = IEEE754<T>::precision / 2;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  const auto ranges_tpl =
      take_n_first<Op::arity>(range_1st, range_2nd, range_3rd);
  const std::array<Range, Op::arity> ranges = tuple_to_array(ranges_tpl);
  using args_t = typename TupleN<Op::arity, V>::type;
  generic::TestRandom<Op::arity>(
      helper::CheckDistributionResultsWrapper<args_t, M, Op, D>, d, config,
      ranges);
}

} // namespace prism::tests::helper::distribution::HWY_NAMESPACE

namespace prism::tests::helper::HWY_NAMESPACE {

}; // namespace prism::tests::helper::HWY_NAMESPACE

HWY_AFTER_NAMESPACE();

#endif // PRISM_TESTS_HELPER_TESTS_TOGGLE