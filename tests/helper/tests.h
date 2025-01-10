#ifndef __PRISM_TESTS_HELPER_TESTS_H__
#define __PRISM_TESTS_HELPER_TESTS_H__

#include "tests/helper/operator.h"
#include <any>
#include <array>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "tests/helper/common.h"
#include "tests/helper/random.h"

namespace prism::tests::helper {

constexpr auto default_repetitions = 100;

template <typename T> auto SimpleCase() -> std::vector<T> {
  std::vector<T> simple_case = {0.0,
                                1.0,
                                2.0,
                                std::numeric_limits<T>::min(),
                                std::numeric_limits<T>::lowest(),
                                std::numeric_limits<T>::max(),
                                std::numeric_limits<T>::epsilon(),
                                std::numeric_limits<T>::infinity(),
                                std::numeric_limits<T>::denorm_min(),
                                std::numeric_limits<T>::signaling_NaN()};
  return simple_case;
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestBasic(TestFunc &&test) {
  const auto simple_case = helper::SimpleCase<T>();
  if constexpr (arity == 1) {
    for (auto a : simple_case) {
      test(a);
      test(-a);
    }
  } else if constexpr (arity == 2) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        test(a, b);
        test(a, -b);
        test(-a, b);
        test(-a, -b);
      }
    }
  } else if constexpr (arity == 3) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        for (auto c : simple_case) {
          test(a, b, c);
          test(a, b, -c);
          test(a, -b, c);
          test(a, -b, -c);
          test(-a, b, c);
          test(-a, b, -c);
          test(-a, -b, c);
          test(-a, -b, -c);
        }
      }
    }
  }
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestBinade(TestFunc &&test, int n) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    if constexpr (arity == 1) {
      T a = rng();
      test(a);
      test(-a);
    } else if constexpr (arity == 2) {
      T a = rng();
      T b = rng();
      test(a, b);
      test(a, -b);
      test(-a, b);
      test(-a, -b);
    } else if constexpr (arity == 3) {
      T a = rng();
      T b = rng();
      T c = rng();
      test(a, b, c);
      test(a, b, -c);
      test(a, -b, c);
      test(a, -b, -c);
      test(-a, b, c);
      test(-a, b, -c);
      test(-a, -b, c);
      test(-a, -b, -c);
    }
  }
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestAllBinades(TestFunc &&test) {
  constexpr auto min_exp = IEEE754<T>::min_exponent_subnormal;
  constexpr auto max_exp = IEEE754<T>::max_exponent;
  for (int i = min_exp; i < max_exp; i++) {
    TestBinade<T, arity>(std::forward<TestFunc>(test), i);
  }
}

template <typename T, int repetitions, typename TestFunc, typename... Ranges>
void TestRandom(TestFunc &&test, Ranges... ranges) {

  static_assert((std::is_same_v<Ranges, Range> && ...),
                "All ranges must be of type Range");

  constexpr auto arity = sizeof...(ranges);

  std::array<Range, arity> ranges_array = {ranges...};

  if constexpr (arity == 1) {
    const auto &range = ranges_array[0];
    RNG rng(range.start, range.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng();
      test(a);
      test(-a);
    }
  } else if constexpr (arity == 2) {
    const auto &range1 = ranges_array[0];
    const auto &range2 = ranges_array[1];
    RNG rng1(range1.start, range1.end);
    RNG rng2(range2.start, range2.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      T b = rng2();
      test(a, b);
      test(a, -b);
      test(-a, b);
      test(-a, -b);
    }
  } else if constexpr (arity == 3) {
    const auto &range1 = ranges_array[0];
    const auto &range2 = ranges_array[1];
    const auto &range3 = ranges_array[2];
    RNG rng1(range1.start, range1.end);
    RNG rng2(range2.start, range2.end);
    RNG rng3(range3.start, range3.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      T b = rng2();
      T c = rng3();
      test(a, b, c);
      test(a, b, -c);
      test(a, -b, c);
      test(a, -b, -c);
      test(-a, b, c);
      test(-a, b, -c);
      test(-a, -b, c);
      test(-a, -b, -c);
    }
  }
}

template <typename T, int repetitions, typename TestFunc, typename Args>
void unpack(TestFunc &&test, Args args) {
  std::apply(
      [&](const auto &...unpacked_args) {
        TestRandom<T, repetitions>(std::forward<TestFunc>(test),
                                   unpacked_args...);
      },
      args);
}

template <size_t N, typename... Args, typename T = std::common_type_t<Args...>>
constexpr auto take_n_first(Args &&...args) {
  if constexpr (N == 0) {
    return std::make_tuple();
  } else {
    constexpr auto idx = N - 1;
    return std::tuple_cat(
        take_n_first<idx>(args...),
        std::make_tuple(std::get<idx>(std::forward_as_tuple(args...))));
  }
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestRandom01(TestFunc &&test) {
  const auto range = Range{0, 1};
  std::array<Range, arity> ranges;
  ranges.fill(range);
  unpack<T, repetitions>(test, ranges);
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestRandomNoOverlap(TestFunc &&test) {

  constexpr auto start2 = IEEE754<T>::ulp / 2;
  constexpr auto end2 = IEEE754<T>::ulp / 4;
  constexpr auto start3 = IEEE754<T>::ulp / 4;
  constexpr auto end3 = IEEE754<T>::ulp / 8;

  const auto range1 = Range{1, 2};
  const auto range2 = Range{start2, end2};
  const auto range3 = Range{start3, end3};

  const auto ranges = take_n_first<arity>(range1, range2, range3);
  unpack<T, repetitions>(test, ranges);
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestRandomLastBitOverlap(TestFunc &&test) {

  constexpr auto start1 = 1;
  constexpr auto end1 = 2;
  constexpr auto start2 = 2 * IEEE754<T>::ulp;
  constexpr auto end2 = IEEE754<T>::ulp;
  constexpr auto start3 = 2 * IEEE754<T>::ulp;
  constexpr auto end3 = IEEE754<T>::ulp;

  const auto range1 = Range{start1, end1};
  const auto range2 = Range{start2, end2};
  const auto range3 = Range{start3, end3};

  const auto ranges = take_n_first<arity>(range1, range2, range3);
  unpack<T, repetitions>(test, ranges);
}

template <typename T, int arity, int repetitions = default_repetitions,
          typename TestFunc = std::any>
void TestRandomMidOverlap(TestFunc &&test) {

  constexpr auto start1 = 1;
  constexpr auto end1 = 2;
  constexpr auto start2 = 2 * IEEE754<T>::ulp;
  constexpr auto end2 = IEEE754<T>::ulp / 2;
  constexpr auto start3 = 2 * IEEE754<T>::ulp;
  constexpr auto end3 = IEEE754<T>::ulp / 4;

  const auto range1 = Range{start1, end1};
  const auto range2 = Range{start2, end2};
  const auto range3 = Range{start3, end3};

  const auto ranges = take_n_first<arity>(range1, range2, range3);
  unpack<T, repetitions>(test, ranges);
}

}; // namespace prism::tests::helper

#if defined(PRISM_TESTS_HELPER_TESTS_H) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_TESTS_HELPER_TESTS_H
#undef PRISM_TESTS_HELPER_TESTS_H
#else
#define PRISM_TESTS_HELPER_TESTS_H
#endif

#include "hwy/highway.h"

#include "src/utils.h"
#include "tests/helper/assert.h"
#include "tests/helper/random.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::tests::helper::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestExactAdd(const D d, ConfigTest &config) {

  constexpr int32_t mantissa = prism::utils::IEEE754<T>::mantissa;
  constexpr auto repetitions = 5;

  for (auto i = 0; i <= repetitions; i++) {
    auto va = hn::Set(d, 1.25);
    auto vb = hn::Set(d, std::ldexp(1.0, -(mantissa + i)));
    CheckDistributionResults<M, Op>(d, config, va, vb);
  }
}

// TODO(yohan): Parallelize this function
template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestSimpleCase(D d, ConfigTest &config) {

  const auto simple_case = SimpleCase<T>();

  if constexpr (Op::arity == 1) {
    for (auto a : simple_case) {
      auto va = hn::Set(d, a);
      CheckDistributionResults<M, Op>(d, config, va);
    }
  } else if constexpr (Op::arity == 2) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        auto va = hn::Set(d, a);
        auto vb = hn::Set(d, b);
        CheckDistributionResults<M, Op>(d, config, va, vb);
      }
    }
  } else if constexpr (Op::arity == 3) {
    for (auto a : simple_case) {
      for (auto b : simple_case) {
        for (auto c : simple_case) {
          auto va = hn::Set(d, a);
          auto vb = hn::Set(d, b);
          auto vc = hn::Set(d, c);
          CheckDistributionResults<M, Op>(d, config, va, vb, vc);
        }
      }
    }
  }
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandom(D d, ConfigTest &config, const Range range_1st = {},
                const Range range_2nd = {}, const Range range_3rd = {}) {

  constexpr auto repetitions = 10;

  if constexpr (Op::arity == 1) {
    RNG rng1(range_1st.start, range_1st.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      auto va = hn::Set(d, a);
      CheckDistributionResults<M, Op>(d, config, va);
      CheckDistributionResults<M, Op>(d, config, hn::Neg(va));
    }
  } else if constexpr (Op::arity == 2) {
    RNG rng1(range_1st.start, range_1st.end);
    RNG rng2(range_2nd.start, range_2nd.end);
    for (int i = 0; i < repetitions; i++) {
      T a = rng1();
      T b = rng2();
      auto va = hn::Set(d, a);
      auto vb = hn::Set(d, b);
      CheckDistributionResults<M, Op>(d, config, va, vb);
      CheckDistributionResults<M, Op>(d, config, va, hn::Neg(vb));
    }
  } else if constexpr (Op::arity == 3) {
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
      CheckDistributionResults<M, Op>(d, config, va, vb, vc);
      CheckDistributionResults<M, Op>(d, config, va, hn::Neg(vb), vc);
      CheckDistributionResults<M, Op>(d, config, va, vb, hn::Neg(vc));
    }
  }
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomNoOverlap(D d, ConfigTest &config) {

  constexpr auto s2 = IEEE754<T>::precision - 1;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  TestRandom<M, Op>(d, config, range_1st, range_2nd, range_3rd);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomLastBitOverlap(D d, ConfigTest &config) {
  constexpr auto s2 = IEEE754<T>::precision;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  TestRandom<M, Op>(d, config, range_1st, range_2nd, range_3rd);
}

template <class M, class Op, class D, class V = hn::VFromD<D>,
          typename T = hn::TFromD<D>>
void TestRandomMidOverlap(D d, ConfigTest &config) {
  constexpr auto s2 = IEEE754<T>::precision / 2;
  const auto _start = [](int s) -> double { return std::ldexp(1.0, s + 1); };
  const auto _end = [](int s) -> double { return std::ldexp(1.0, s + 2); };

  const Range range_1st{1, 2};
  const Range range_2nd{_start(s2), _end(s2)};
  const Range range_3rd{_start(s2), _end(s2)};

  TestRandom<M, Op>(d, config, range_1st, range_2nd, range_3rd);
}

}; // namespace prism::tests::helper::HWY_NAMESPACE

HWY_AFTER_NAMESPACE();

#endif // PRISM_TESTS_HELPER_TESTS_H

#endif // __PRISM_TESTS_HELPER_TESTS_H__