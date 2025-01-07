#ifndef __PRISM_TESTS_HELPER_H__
#define __PRISM_TESTS_HELPER_H__

#include <array>
#include <cstdio>
#include <functional>
#include <map>
#include <random>

#include <boost/math/distributions/binomial.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int/literals.hpp>

#include "src/utils.h"

namespace helper {

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

template <typename T> struct Operator {
public:
  enum operator_type {};
  using function = void;
  static constexpr auto ftype = FTypeName<T>::name;
};

template <typename T> struct UnaryOperator : Operator<T> {
public:
  enum operator_type { SQRT, ABS, PREDECESSOR, SUCCESSOR };
  using function = std::function<T(T)>;
  using function_ptr = std::function<T (*)(T)>;
};

template <typename T> struct BinaryOperator : Operator<T> {
public:
  enum operator_type { ADD, SUB, MUL, DIV };
  using function = std::function<T(T, T)>;
  using function_ptr = std::function<T (*)(T, T)>;
};

template <typename T> struct TernaryOperator : Operator<T> {
public:
  enum operator_type { FMA };
  using function = std::function<T(T, T, T)>;
  using function_ptr = std::function<T (*)(T, T, T)>;
};

template <typename T> struct SqrtOp : public UnaryOperator<T> {
  using function = typename UnaryOperator<T>::function;
  using function_ptr = typename UnaryOperator<T>::function_ptr;
  using operator_type = typename UnaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::SQRT;
  static constexpr auto name = "sqrt";
  static constexpr auto symbol = "√";
};

template <typename T> struct AddOp : public BinaryOperator<T> {
  using function = typename BinaryOperator<T>::function;
  using function_ptr = typename BinaryOperator<T>::function_ptr;
  using operator_type = typename BinaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::ADD;
  static constexpr auto name = "add";
  static constexpr auto symbol = "+";
};

template <typename T> struct SubOp : BinaryOperator<T> {
  using function = typename BinaryOperator<T>::function;
  using function_ptr = typename BinaryOperator<T>::function_ptr;
  using operator_type = typename BinaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::SUB;
  static constexpr auto name = "sub";
  static constexpr auto symbol = "-";
};

template <typename T> struct MulOp : BinaryOperator<T> {
  using function = typename BinaryOperator<T>::function;
  using function_ptr = typename BinaryOperator<T>::function_ptr;
  using operator_type = typename BinaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::MUL;
  static constexpr auto name = "mul";
  static constexpr auto symbol = "*";
};
template <typename T> struct DivOp : BinaryOperator<T> {
  using function = typename BinaryOperator<T>::function;
  using function_ptr = typename BinaryOperator<T>::function_ptr;
  using operator_type = typename BinaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::DIV;
  static constexpr auto name = "div";
  static constexpr auto symbol = "/";
};

template <typename T> struct FmaOp : TernaryOperator<T> {
  using function = typename TernaryOperator<T>::function;
  using function_ptr = typename TernaryOperator<T>::function_ptr;
  using operator_type = typename TernaryOperator<T>::operator_type;
  static constexpr auto type = operator_type::FMA;
  static constexpr auto name = "fma";
  static constexpr auto symbol = "fma";
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

template <typename T> auto hexfloat(const T &a) -> std::string {
  if constexpr (std::is_same<T, Float128_boost>::value) {
    return hexfloat(a);
  } else if constexpr (std::is_same<T, float>::value) {
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

template <typename T> auto absolute_distance(const T a, const T b = 0) -> T {
  if constexpr (std::is_same<T, Float128_boost>::value) {
    return boost::multiprecision::abs(a - b);
  } else {
    return std::abs(a - b);
  }
}

template <typename T> auto relative_distance(const T a, const T b) -> T {
  // relative distance between a and b
  // return b if a is zero
  // return a if b is zero
  // return |a - b| / |a| otherwise
  if (a == 0) {
    return b;
  }
  if (b == 0) {
    return a;
  }
  return absolute_distance(a, b) / absolute_distance(a);
}

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

template <typename T> auto isfinite(T a) -> bool {
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

struct RNG {
private:
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

public:
  explicit RNG(double a = 0.0, double b = 1.0) : gen(RNG::rd()), dis(a, b){};
  auto operator()() -> double { return dis(gen); }
};

template <typename T>
void test_binade(int n, std::function<void(T, T)> &test,
                 int repetitions = 100) {
  auto start = std::ldexp(1.0, n);
  auto end = std::ldexp(1.0, n + 1);
  RNG rng(start, end);

  for (int i = 0; i < repetitions; i++) {
    T a = rng();
    T b = rng();
    test(a, b);
    test(a, -b);
    test(-a, b);
    test(-a, -b);
  }
}

template <typename T> class Counter {
private:
  int _up_count{};
  int _down_count{};
  T _down;
  T _up;
  std::map<T, int> data;
  bool _is_finalized = false;

public:
  Counter() = default;

  auto operator[](const T &key) -> int & {
    _is_finalized = false;
    return data[key];
  }

  void finalize() {
    if (_is_finalized) {
      return;
    }

    auto keys = data.begin();
    _down = keys->first;
    _down_count = keys->second;
    keys++;
    if (keys == data.end()) {
      _is_finalized = true;
      _up = 0;
      _up_count = 0;
      return;
    }
    _up = keys->first;
    _up_count = keys->second;

    if (_down > _up) {
      std::swap(_down, _up);
      std::swap(_down_count, _up_count);
    }

    _is_finalized = true;
  }

  [[nodiscard]] auto size() const -> int { return data.size(); }

  auto down() -> const T & {
    finalize();
    return _down;
  }
  auto up() -> const T & {
    finalize();
    return _up;
  }

  auto down_count() -> const int & {
    finalize();
    return _down_count;
  }

  auto up_count() -> const int & {
    finalize();
    return _up_count;
  }
};

struct BinomialTest {
  double lower;
  double upper;
  double pvalue;
};

inline auto binomial_test(const int n, const int k,
                          const double p) -> BinomialTest {

  boost::math::binomial_distribution<> dist(n, p);
  double lower = boost::math::cdf(dist, k);

  double upper{};
  double pvalue{};
  if (k > 0) {
    upper = boost::math::cdf(complement(dist, k - 1));
    pvalue = 2 * std::min(lower, upper);
  } else {
    upper = 1;
    pvalue = 2 * lower;
  }

  return BinomialTest{lower, upper, pvalue};
}

namespace reference {
// return pred(|s|)

// compute in double precision if the input type is float
// compute in quad precision if the input type is double
template <typename T, typename H = typename helper::IEEE754<T>::H>
auto add(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a + b;
}

template <typename T, typename H = typename helper::IEEE754<T>::H>
auto sub(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a - b;
}

template <typename T, typename H = typename helper::IEEE754<T>::H>
auto mul(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a * b;
}

template <typename T, typename H = typename helper::IEEE754<T>::H>
auto div(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  return a / b;
}

template <typename T, typename H = typename helper::IEEE754<T>::H>
auto sqrt(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  return helper::sqrt<H>(a);
}

template <typename T, typename H = typename helper::IEEE754<T>::H>
auto fma(const std::vector<T> &args) -> H {
  auto a = static_cast<H>(args[0]);
  auto b = static_cast<H>(args[1]);
  auto c = static_cast<H>(args[2]);
  return helper::fma<H>(a, b, c);
}

}; // namespace reference

template <typename Op>
struct PrAdd {
  static constexpr char name[] = "add";
  static constexpr char symbol[] = "+";
  static constexpr int arity = 2;

  template <typename T> auto operator()(T a, T b) -> T { return Op::add(a, b); }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static auto reference(const std::vector<T> &args) -> H {
    return helper::reference::add<T>(args);
  }
};

struct SRSub {
  static constexpr char name[] = "sub";
  static constexpr char symbol[] = "-";
  static constexpr int arity = 2;

  template <typename T> auto operator()(T a, T b) -> T { return sr::sub(a, b); }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static auto reference(const std::vector<T> &args) -> H {
    return reference::sub<T>(args);
  }
};

struct SRMul {
  static constexpr char name[] = "mul";
  static constexpr char symbol[] = "*";
  static constexpr int arity = 2;

  template <typename T> auto operator()(T a, T b) -> T { return sr::mul(a, b); }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static auto reference(const std::vector<T> &args) -> H {
    return reference::mul<T>(args);
  }
};

struct SRDiv {
  static constexpr char name[] = "div";
  static constexpr char symbol[] = "/";
  static constexpr int arity = 2;

  template <typename T> auto operator()(T a, T b) -> T { return sr::div(a, b); }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static H reference(const std::vector<T> &args) {
    return reference::div<T>(args);
  }
};

struct SRSqrt {
  static constexpr char name[] = "sqrt";
  static constexpr char symbol[] = "√";
  static constexpr int arity = 1;

  template <typename T> auto operator()(T a) -> T { return sr::sqrt(a); }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static H reference(const std::vector<T> &args) {
    return reference::sqrt<T>(args);
  }
};

struct SRFma {
  static constexpr char name[] = "fma";
  static constexpr char symbol[] = "fma";
  static constexpr int arity = 3;

  template <typename T> auto operator()(T a, T b, T c) -> T {
    return sr::fma(a, b, c);
  }

  template <typename T, typename H = typename helper::IEEE754<T>::H>
  static H reference(const std::vector<T> &args) {
    return reference::fma<T>(args);
  }
};

}; // namespace helper

#endif // __PRISM_TESTS_HELPER_H__