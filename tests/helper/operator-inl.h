
#if defined(PRISM_TEST_HELPER_OPERATOR_TOGGLE) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_TEST_HELPER_OPERATOR_TOGGLE
#undef PRISM_TEST_HELPER_OPERATOR_TOGGLE
#else
#define PRISM_TEST_HELPER_OPERATOR_TOGGLE
#endif

#include <iostream>

#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::tests::helper::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
inline auto reduce_min(D d, const V &v) -> T {
#if HWY_TARGET == HWY_EMU128
  const auto N = hn::Lanes(d);
  T _min = std::numeric_limits<T>::infinity();
  for (auto i = 0; i < N; i++) {
    const auto lane = hn::ExtractLane(v, i);
    _min = std::isnan(lane) ? lane : std::min(_min, lane);
  }
  return _min;
#else
  return hn::ReduceMin(d, v);
#endif
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
inline auto reduce_max(D d, const V &v) -> T {
#if HWY_TARGET == HWY_EMU128
  const auto N = hn::Lanes(d);
  T _max = -std::numeric_limits<T>::infinity();
  for (auto i = 0; i < N; i++) {
    const auto lane = hn::ExtractLane(v, i);
    _max = std::isnan(lane) ? lane : std::max(_max, lane);
  }
  return _max;
#else
  return hn::ReduceMax(d, v);
#endif
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
inline auto extract_unique_lane(D d, V v) -> T {
  const T _min = reduce_min(d, v);
  const T _max = reduce_max(d, v);
  if (_min != _max and not(isnan(_min) and isnan(_max))) {
    std::cerr << "Failed for\n"
              << "min: " << _min << "\n"
              << "max: " << _max << std::endl;

    const auto N = hn::Lanes(d);
    for (auto i = 0; i < N; i++) {
      std::cerr << "lane[" << i << "]: " << hn::ExtractLane(v, i) << std::endl;
    }
    HWY_ASSERT(_min == _max); // NOLINT
  }
  return _min;
}
}; // namespace prism::tests::helper::HWY_NAMESPACE

HWY_AFTER_NAMESPACE(); // at file scope

#endif // PRISM_TEST_HELPER_OPERATOR_TOGGLE