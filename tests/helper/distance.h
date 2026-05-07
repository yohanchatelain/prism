#ifndef __PRISM_TESTS_HELPER_DISTANCE_H__
#define __PRISM_TESTS_HELPER_DISTANCE_H__

#include <string>

#include "tests/helper/operator.h"

namespace prism::tests::helper {

template <typename H> struct DistanceError {
  H reference;
  H error;
  H error_c;
  H probability_down;
  H probability_up;
  H next;
  H prev;
  H ulp;
  int exponent_prev{};
  int exponent_next{};
  std::string msg;
  bool is_exact{};

public:
  template <typename T> void set_distance_error(T /* unused */) {
    const auto ref_cast = static_cast<T>(reference);
    std::ostringstream os;
    os << std::hexfloat << std::setprecision(13);
    os << "-- compute_distance_error --" << std::endl;
    os << "         reference: " << hexfloat(reference) << std::endl;
    if constexpr (std::is_same_v<T, float>) {
      os << "  (float)reference: " << hexfloat(ref_cast) << std::endl;
      os << "  |ref-(float)ref|: " << hexfloat(error) << std::endl;
    }
    if constexpr (std::is_same_v<T, double>) {
      os << " (double)reference: " << hexfloat(ref_cast) << std::endl;
      os << " |ref-(double)ref|: " << hexfloat(error) << std::endl;
    }
    os << "           error_c: " << hexfloat(error_c) << std::endl;
    os << "               ulp: " << hexfloat(ulp) << std::endl;
    os << "       reference ↓: " << hexfloat(prev) << std::endl;
    os << "       reference ↑: " << hexfloat(next) << std::endl;
    os << "        exponent ↓: " << exponent_prev << std::endl;
    os << "        exponent ↑: " << exponent_next << std::endl;
    os << std::defaultfloat;
    os << "                 p: " << probability_down << std::endl;
    os << "               1-p: " << probability_up << std::endl;
    msg = os.str();
  }
};

template <typename T1, typename T2 = T1>
auto absolute_distance(const T1 a,
                       const T2 b = 0) -> std::common_type_t<T1, T2> {
  using CommonType = std::common_type_t<T1, T2>;
  if constexpr (std::is_same<CommonType, Float128_boost>::value) {
    return boost::multiprecision::abs(static_cast<CommonType>(a) -
                                      static_cast<CommonType>(b));
  } else {
    return std::abs(static_cast<CommonType>(a) - static_cast<CommonType>(b));
  }
}

template <typename T1, typename T2>
auto relative_distance(const T1 a, const T2 b) -> std::common_type_t<T1, T2> {
  // relative distance between a and b
  // return b if a is zero
  // return a if b is zero
  // return |a - b| / |a| otherwise
  using CommonType = std::common_type_t<T1, T2>;
  if (a == 0) {
    return static_cast<CommonType>(b);
  }
  if (b == 0) {
    return static_cast<CommonType>(a);
  }
  return absolute_distance(a, b) / absolute_distance(a);
}

template <typename Op = void, typename T, typename H = typename IEEE754<T>::H>
auto is_exact_operation(Args<T> args, const H reference) -> bool {
  T ref_cast = static_cast<T>(reference);

  bool is_exact = false;

  // if any of the arguments is nan or inf, the result is exact
  for (const auto &a : args) {
    is_exact |= isnan(a);
    is_exact |= isinf(a);
  }

  // if the result is nan or inf, the result is exact
  is_exact |= isnan(reference);
  is_exact |= isnan(ref_cast);
  is_exact |= isinf(reference);
  is_exact |= isinf(ref_cast);

  // Check exactness at the virtual precision, not just hardware precision.  If
  // an operation is exact in hardware but falls between virtual
  // representatives, it must still be stochastically rounded.
  if (!is_exact) {
    if (prism::sr::get_virtual_precision<T>() < IEEE754<T>::mantissa) {
      // Truncate ref_cast to virtual precision and check if it matches reference
      is_exact = (static_cast<H>(prism::sr::truncate_mantissa(ref_cast, prism::sr::get_virtual_precision<T>())) == reference);
    } else {
      is_exact = (ref_cast - reference) == 0;
    }
  }

  return is_exact;
}

template <typename Op = void, typename T, typename H = typename IEEE754<T>::H>
auto compute_distance_error(Args<T> args, H reference) -> DistanceError<H> {
  T ref_cast = static_cast<T>(reference);
  if (prism::sr::get_virtual_precision<T>() < IEEE754<T>::mantissa) {
    ref_cast = prism::sr::truncate_mantissa(ref_cast, prism::sr::get_virtual_precision<T>());
  }

  DistanceError<H> result = {.reference = reference,
                             .error = 0,
                             .error_c = 0,
                             .probability_down = 0,
                             .probability_up = 0,
                             .next = 0,
                             .prev = 0,
                             .ulp = get_ulp(ref_cast),
                             .exponent_prev = 0,
                             .exponent_next = 0,
                             .msg = "Not initialized",
                             .is_exact = false};

  if (is_exact_operation<Op>(args, reference)) {
    result.is_exact = true;
    result.probability_down = 1;
    result.msg = "Exact operation";
    return result;
  }

  result.error = absolute_distance(reference, ref_cast);
  result.error_c = absolute_distance(result.ulp, result.error);
  result.prev = (ref_cast < reference) ? ref_cast : (ref_cast - result.ulp);
  result.next = (ref_cast < reference) ? (ref_cast + result.ulp) : ref_cast;
  result.probability_down = (result.next - reference) / result.ulp;
  result.probability_up = (reference - result.prev) / result.ulp;

  result.exponent_next = get_exponent(result.next);
  result.exponent_prev = get_exponent(result.prev);

  const bool same_binade = result.exponent_next == result.exponent_prev;

  // if the float128 error is smaller 2^-52 * result.ulp; then the probability of the
  // casted reference being the next representable value is equal to 1
  const bool rel_error_small = result.error < IEEE754<T>::ulp * result.ulp;

  // Fasi and Mikaitis note that twodiv and twosqrt are approximations
  // eg. with 1.0 / 2^128, unscaled twodiv will flush tau to zero.
  // For twodiv, we could conditionally scale to fix this, but it's a performance tradeoff.
  // here we stick with the twodiv and twosqrt proposed in Fasi and Mikaitis paper
  bool eft_underflow = false;
  if constexpr (!std::is_void_v<Op> && std::is_base_of_v<PrDiv, Op>) {
    // tau is too small
    const bool final_underflow = result.error < IEEE754<T>::min_subnormal;
    // twodiv FMA (tau * b) is flushed
    const auto approx_remainder = result.error * std::abs(args[1]);
    const bool intermediate_underflow =
        approx_remainder < IEEE754<T>::min_subnormal;
    eft_underflow = final_underflow || intermediate_underflow;
  } else if constexpr (!std::is_void_v<Op> && std::is_base_of_v<PrSqrt, Op>) {
    // twosqrt FMA approximation: r ~ tau * 2 * sigma is flushed
    const auto approx_remainder =
        (result.error * T(2.0) * result.ulp) / IEEE754<T>::ulp;
    eft_underflow = approx_remainder < IEEE754<T>::min_subnormal;
  }

  // ignore exact results and twodiv / twosqrt underflows
  if (rel_error_small || eft_underflow) {
    result.is_exact = true;
  }

  if (result.is_exact) {
  } else if (not same_binade) {
    const auto ordered = (result.exponent_next < result.exponent_prev);
    H ulp_prev = ordered ? result.ulp : (result.ulp / 2);
    H ulp_next = ordered ? (result.ulp / 2) : result.ulp;
    result.prev = (ref_cast < reference) ? ref_cast : (ref_cast - ulp_prev);
    result.next = (ref_cast < reference) ? (ref_cast + ulp_next) : ref_cast;
    // For variable precision crossing a binade boundary, the distances might be
    // asymmetric so we cannot assume fixed 0.5/0.5 probabilities.
    H distance = result.next - result.prev;
    result.probability_down = (result.next - reference) / distance;
    result.probability_up = (reference - result.prev) / distance;
  }

  result.set_distance_error(T{});

  return result;
}

}; // namespace prism::tests::helper

#endif // __PRISM_TESTS_HELPER_DISTANCE_H__