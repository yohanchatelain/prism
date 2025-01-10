#ifndef __PRISM_TESTS_HELPER_COUNTER_H
#define __PRISM_TESTS_HELPER_COUNTER_H

#include <map>

namespace prism::tests::helper {

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
  [[nodiscard]] auto count() const -> int { return _down_count + _up_count; }

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
}; // namespace prism::tests::helper

#endif // PRISM_TESTS_HELPER_COUNTER_H