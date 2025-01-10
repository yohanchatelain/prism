#ifndef __PRISM_TESTS_HELPER_PPRINT_H__
#define __PRISM_TESTS_HELPER_PPRINT_H__

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "src/utils.h"
#include "tests/helper/operator.h"

namespace prism::tests::helper {

template <typename T> auto fmt_proba(const T &x) -> std::string {
  std::ostringstream os;
  const auto precision = 5;
  os << std::fixed << std::setprecision(precision) << x << std::defaultfloat;
  return os.str();
}

template <typename T, typename Op,
          typename H = typename prism::utils::IEEE754<T>::H>
auto get_args_str(Args<T> args, H reference) -> std::string {
  const auto symbol = std::string(Op::symbol);

  std::string symbol_op;
  std::string args_str;
  if constexpr (Op::arity == 1) {
    auto a = args[0];
    symbol_op = "     " + symbol + "a: ";
    args_str = "               a: " + hexfloat(a) + "\n" + symbol_op +
               hexfloat(reference) + "\n";
  } else if constexpr (Op::arity == 2) {
    auto a = args[0];
    auto b = args[1];
    symbol_op = "             a" + symbol + "b: ";
    args_str = "               a: " + hexfloat(a) + "\n" +
               "               b: " + hexfloat(b) + "\n" + symbol_op +
               hexfloat(reference) + "\n";
  } else if constexpr (Op::arity == 3) {
    auto a = args[0];
    auto b = args[1];
    auto c = args[2];
    symbol_op = "     " + symbol + "(a, b, c): ";
    args_str = "               a: " + hexfloat(a) + "\n" +
               "               b: " + hexfloat(b) + "\n" +
               "               c: " + hexfloat(c) + "\n" + symbol_op +
               hexfloat(reference) + "\n";
  }
  return args_str;
}

}; // namespace prism::tests::helper

#endif // __PRISM_TESTS_HELPER_PPRINT_H__