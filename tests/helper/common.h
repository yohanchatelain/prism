#ifndef __PRISM_TESTS_HELPER_COMMON_H___
#define __PRISM_TESTS_HELPER_COMMON_H___

#include <algorithm>
#include <cctype>
#include <string>
#include <type_traits>

namespace prism::tests::helper {

struct RoundingMode {
  struct UD {
    static constexpr auto is_ud = true;
    static constexpr auto is_sr = false;
  };
  struct SR {
    static constexpr auto is_ud = false;
    static constexpr auto is_sr = true;
  };

  template <typename T>
  static constexpr auto is_rounding_mode =
      std::is_same_v<T, UD> or std::is_same_v<T, SR>;

  template <typename T> static constexpr void static_assert_rounding_mode() {
    static_assert(is_rounding_mode<T>, "T must be a member of RoundingMode");
  }

  template <typename T>
  static constexpr auto is_ud = is_rounding_mode<T> and T::is_ud;
  template <typename T>
  static constexpr auto is_sr = is_rounding_mode<T> and T::is_sr;
};

struct Range {
  double start{0};
  double end{1};
};

struct ConfigTest {
  std::string name;
  std::string description;
  int repetitions;
  int distribution_tests_counter;
  double alpha;
};

inline auto to_lower(const std::string &input) -> std::string {
  auto output = input;
  std::transform(output.begin(), output.end(), output.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return output;
}

struct Operator {
  enum class Type { Add, Sub, Mul, Div, Sqrt, Fma, Unknown };

  explicit Operator(const std::string &name) {
    const auto &name_lower = to_lower(name);
    if (name == "add") {
      _type = Type::Add;
    } else if (name == "sub") {
      _type = Type::Sub;
    } else if (name == "mul") {
      _type = Type::Mul;
    } else if (name == "div") {
      _type = Type::Div;
    } else if (name == "sqrt") {
      _type = Type::Sqrt;
    } else if (name == "fma") {
      _type = Type::Fma;
    } else {
      _type = Type::Unknown;
    }
  }

  [[nodiscard]] auto type() const -> Type { return _type; }

  [[nodiscard]] auto name() const -> std::string {
    switch (_type) {
    case Type::Add:
      return "Add";
    case Type::Sub:
      return "Sub";
    case Type::Mul:
      return "Mul";
    case Type::Div:
      return "Div";
    case Type::Sqrt:
      return "Sqrt";
    case Type::Fma:
      return "FMA";
    default:
      return "Unknown";
    }
  }

  [[nodiscard]] auto arity() const -> int {
    switch (_type) {
    case Type::Add:
    case Type::Sub:
    case Type::Mul:
    case Type::Div:
      return 2;
    case Type::Sqrt:
      return 1;
    case Type::Fma:
      return 3;
    default:
      return 0;
    }
  }

private:
  Type _type = Type::Unknown;
};

template <size_t N, typename T> struct TupleN {
  template <size_t i> static constexpr auto tupleN_impl(const T &t = T{}) {
    static_assert(N <= 3, "Only 3 args are supported");
    if constexpr (i == 0) {
      return std::make_tuple();
    } else {
      return std::tuple_cat(tupleN_impl<i - 1>(), std::make_tuple(t));
    }
  }
  using type = decltype(tupleN_impl<N>());

  auto get(T t) const { return tupleN_impl<N>(t); }
};

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

template <typename Tuple, std::size_t... I>
auto tuple_to_array_impl(const Tuple &t, std::index_sequence<I...>) {
  return std::array{std::get<I>(t)...};
}

template <typename... T> auto tuple_to_array(const std::tuple<T...> &t) {
  return tuple_to_array_impl(t, std::index_sequence_for<T...>{});
}

} // namespace prism::tests::helper

#endif // PRISM_TESTS_HELPER_COMMON_H_