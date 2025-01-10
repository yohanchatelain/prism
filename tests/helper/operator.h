
#ifndef __PRISM_TESTS_HELPER_OPERATOR_H__
#define __PRISM_TESTS_HELPER_OPERATOR_H__

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int/literals.hpp>

#include "src/debug.h"
#include "src/utils.h"
#include "tests/helper/counter.h"

namespace prism::tests::helper {

template <typename T> using Args = const std::vector<T> &;

/* Operators */

template <typename T> struct FTypeName {
  static constexpr auto name = "unknown";
};

template <> struct FTypeName<float> {
  static constexpr auto name = "float";
};

template <> struct FTypeName<double> {
  static constexpr auto name = "double";
};

using Float128_boost = boost::multiprecision::cpp_bin_float_quad;

template <typename T> struct IEEE754 : prism::utils::IEEE754<T> {
  using H =
      std::conditional_t<std::is_same<T, float>::value, double, Float128_boost>;
};

using boost::multiprecision::literals::operator""_cppui;
template <> struct IEEE754<Float128_boost> {
  using I = boost::multiprecision::int128_t;
  using U = boost::multiprecision::uint128_t;
  using H = Float128_boost;
  static constexpr I sign = 1;
  static constexpr I exponent = 15;
  static constexpr I mantissa = 112;
  static constexpr I precision = 113;
  static constexpr I bias = 0x3FFF;
  static constexpr U exponent_mask = 0x7FFF;
  static constexpr I min_exponent = -16382;
  static constexpr I max_exponent = 16383;
  static constexpr I min_exponent_subnormal = -16495;
  static constexpr U inf_nan_mask = 0x7FFF0000000000000000000000000000_cppui;
};

template <typename T> auto isnan(const T &a) -> bool {
  if constexpr (std::is_same_v<T, Float128_boost>) {
    return boost::multiprecision::isnan(a);
  } else {
    return std::isnan(a);
  }
}

template <typename T> auto isinf(const T &a) -> bool {
  if constexpr (std::is_same_v<T, Float128_boost>) {
    return boost::multiprecision::isinf(a);
  } else {
    return std::isinf(a);
  }
}

template <typename T> auto isfinite(const T a) -> bool {
  return not isnan(a) and not isinf(a);
}

template <typename T> auto abs(const T &a) -> T {
  if constexpr (std::is_same<T, Float128_boost>::value) {
    return boost::multiprecision::abs(a);
  } else {
    return std::abs(a);
  }
}

template <typename T> auto sqrt(const T &a) -> T {
  if constexpr (std::is_same<T, Float128_boost>::value) {
    return boost::multiprecision::sqrt(a);
  } else {
    return std::sqrt(a);
  }
}

template <typename T> auto fma(const T &a, const T &b, const T &c) -> T {
  if constexpr (std::is_same_v<T, Float128_boost>) {
    return boost::multiprecision::fma(a, b, c);
  } else {
    return std::fma(a, b, c);
  }
}

template <typename T> auto is_subnormal(const T a) -> bool {
  return not isnan(a) and not isinf(a) and a != 0 and
         abs(a) < IEEE754<T>::min_normal;
}

template <typename T> auto get_exponent(T a) -> int {
  int exp = 0;
  if (a == 0) {
    return 0;
  }
  if constexpr (std::is_same_v<T, Float128_boost>) {
    const auto res =
        boost::multiprecision::floor(boost::multiprecision::log2(abs(a)));
    return static_cast<int>(res);
  } else {
    using U = typename IEEE754<T>::U;
    U u;
    std::memcpy(&u, &a, sizeof(T));
    u &= IEEE754<T>::exponent_mask_scaled;
    u >>= IEEE754<T>::mantissa;
    exp = static_cast<int>(u);
    exp -= IEEE754<T>::bias;
  }
  return exp;
}

template <typename T> auto is_power_of_2(T a) -> bool {
  if (a == 0) {
    return false;
  }
  if constexpr (std::is_same_v<T, Float128_boost>) {
    return boost::multiprecision::log2(a) ==
           boost::multiprecision::floor(boost::multiprecision::log2(a));
  } else {
    return std::log2(a) == std::floor(std::log2(a));
  }
}

// compute ulp(a)
template <typename T, typename H = typename IEEE754<T>::H>
auto get_ulp(T a) -> H {
  if constexpr (std::is_same_v<T, Float128_boost>) {
    constexpr int mantissa = static_cast<int>(IEEE754<T>::mantissa);
    const int exponent = get_exponent(a);
    H one = 1.0;
    H ulp = boost::multiprecision::ldexp(one, exponent - mantissa);
    return ulp;
  } else {
    if (is_subnormal(a)) {
      return static_cast<H>(IEEE754<T>::min_subnormal);
    }
    int exponent = get_exponent(a);
    H ulp = std::ldexp(1.0, exponent - IEEE754<T>::mantissa);
    return ulp;
  }
}

template <typename T> auto hexfloat(const T &a) -> std::string {
  if constexpr (std::is_same<T, float>::value) {
    constexpr size_t buffer_size = 50;
    std::array<char, buffer_size> fmt = {};
    snprintf(fmt.data(), fmt.size(), "%.6a", a);
    std::string s(fmt.data());
    return s;
  } else {
    constexpr size_t buffer_size = 50;
    std::array<char, buffer_size> fmt = {};
    snprintf(fmt.data(), fmt.size(), "%.13a", a);
    std::string s(fmt.data());
    return s;
    return s;
  }
}

inline auto hexfloat(const Float128_boost &a) -> std::string {
  using u128 = boost::multiprecision::uint128_t;
  std::stringstream ss;
  u128 mantissa = a.backend().bits().limbs()[0];
  const auto *sign = a.backend().sign() ? "-" : "+";
  auto exp = a.backend().exponent();
  u128 mask = 0xf;
  mask <<= 112;
  mantissa &= ~mask;
  if (a == 0) {
    ss << sign << "0.0p0";
  } else {
    ss << sign << "0x1." << std::hex << mantissa << "p" << std::dec << exp;
  }
  ss << std::defaultfloat;
  return ss.str();
}

inline auto flush() -> std::string {
  debug_flush();
  return "";
}

namespace reference {
// return pred(|s|)

// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename IEEE754<T>::H>
auto add(Args<T> args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a + b;
}

template <typename T, typename H = typename IEEE754<T>::H>
auto sub(Args<T> args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a - b;
}

template <typename T, typename H = typename IEEE754<T>::H>
auto mul(Args<T> args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a * b;
}

template <typename T, typename H = typename IEEE754<T>::H>
auto div(Args<T> args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a / b;
}

template <typename T, typename H = typename IEEE754<T>::H>
auto sqrt(Args<T> args) -> H {
  using ::sqrt;
  auto a = static_cast<H>(args[0]);
  return sqrt(a);
}

template <typename T, typename H = typename IEEE754<T>::H>
auto fma(Args<T> args) -> H {
  using ::fma;
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  auto c = static_cast<H>(args[2]);
  return fma(a, b, c);
}

}; // namespace reference

struct PrAdd {
  static constexpr char name[] = "add";
  static constexpr char symbol[] = "+";
  static constexpr int arity = 2;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::add<T>(args);
  }
};

struct PrSub {
  static constexpr char name[] = "sub";
  static constexpr char symbol[] = "-";
  static constexpr int arity = 2;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::sub<T>(args);
  }
};

struct PrMul {
  static constexpr char name[] = "mul";
  static constexpr char symbol[] = "*";
  static constexpr int arity = 2;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::mul<T>(args);
  }
};

struct PrDiv {
  static constexpr char name[] = "div";
  static constexpr char symbol[] = "/";
  static constexpr int arity = 2;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::div<T>(args);
  }
};

struct PrSqrt {
  static constexpr char name[] = "sqrt";
  static constexpr char symbol[] = "√";
  static constexpr int arity = 1;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::sqrt<T>(args);
  }
};

struct PrFma {
  static constexpr char name[] = "fma";
  static constexpr char symbol[] = "fma";
  static constexpr int arity = 3;

  template <typename T, typename H = typename IEEE754<T>::H>
  static auto reference(Args<T> args) -> H {
    return reference::fma<T>(args);
  }
};
}; // namespace prism::tests::helper

// Highway specific functions

#if defined(PRISM_TESTS_HELPER_OPERATOR_H) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_TESTS_HELPER_OPERATOR_H
#undef PRISM_TESTS_HELPER_OPERATOR_H
#else
#define PRISM_TESTS_HELPER_OPERATOR_H
#endif

#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::test::helper::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
auto reduce_min(D d, V v) -> T {
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
auto reduce_max(D d, V v) -> T {
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
auto extract_unique_lane(D d, V v) -> T {
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
    HWY_ASSERT(_min == _max);
  }
  return _min;
}
}; // namespace prism::test::helper::HWY_NAMESPACE

HWY_AFTER_NAMESPACE(); // at file scope

#endif // PRISM_TESTS_HELPER_OPERATOR_H
#endif // PRISM_TESTS_HELPER_OPERATOR_H