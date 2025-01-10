#include <iostream>
#include <vector>

#include "hwy/highway.h"
#include "hwy/print-inl.h"

#include "src/ud_scalar-inl.h"

#include "tests/helper/common.h"
#include "tests/helper/operator.h"

namespace ud = prism::ud::scalar::HWY_NAMESPACE;
namespace helper = prism::tests::helper;
using Op = helper::Operator;

template <typename T> void print(const T a) {
  constexpr const char *fmt = helper::IEEE754<T>::format;
  fprintf(stderr, fmt, a);
}

template <typename T> void test_op(helper::Args<T> args, const Op op) {

  constexpr const char *fptype = std::is_same_v<T, float> ? "f32" : "f64";

  std::cerr << op.name() << fptype << ":\n";

  switch (op.type()) {
  case Op::Type::Add:
    print(ud::add(args[0], args[1]));
    break;
  case Op::Type::Sub:
    print(ud::sub(args[0], args[1]));
    break;
  case Op::Type::Mul:
    print(ud::mul(args[0], args[1]));
    break;
  case Op::Type::Div:
    print(ud::div(args[0], args[1]));
    break;
  case Op::Type::Sqrt:
    print(ud::sqrt(args[0]));
    break;
  case Op::Type::Fma:
    print(ud::fma(args[0], args[1], args[2]));
    break;
  default:
    std::cerr << "Unknown operation: " << op.name() << std::endl;
    break;
  }

  std::cerr << std::endl;
}

template <typename T>
auto handle_args(int argi, int argc, char *argv[],
                 int arity) -> std::vector<T> {
  std::vector<T> args(arity);

  if (argc != argi + arity) {
    std::cerr << "Usage: " << argv[0] << " <type> <op> <args...>\n";
    std::exit(1);
  }

  for (int i = 0; i < arity; i++) {
    args[i] = static_cast<T>(strtod(argv[argi++], NULL));
  }

  return args;
}

auto main(int argc, char *argv[]) -> int {

  int arg_index = 0;

  if (argc <= 2) {
    std::cerr << "Usage: " << argv[arg_index++] << " <type> <op> <args...>\n";
    return 1;
  }
  arg_index++;

  const std::string type_name = std::string(argv[arg_index++]);
  const std::string op_name = std::string(argv[arg_index++]);

  const auto op = Op(op_name);
  if (op.type() == Op::Type::Unknown) {
    std::cerr << "Unknown operation: " << op_name << std::endl;
    return 1;
  }

  if (type_name == "f32") {
    const auto args = handle_args<float>(arg_index, argc, argv, op.arity());
    test_op(args, op);
  } else if (type_name == "f64") {
    const auto args = handle_args<double>(arg_index, argc, argv, op.arity());
    test_op(args, op);
  } else {
    std::cerr << "Unknown type: " << type_name << std::endl;
    return 1;
  }

  return 0;
}