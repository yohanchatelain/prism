#ifndef __PRISM_TESTS_HELPER_COUNTER_H
#define __PRISM_TESTS_HELPER_COUNTER_H

#include <map>

namespace prism::tests::helper {

template <typename T> class Counter {
private:
  std::map<T, int> _data;
  T _prev_expected;
  T _next_expected;

public:
  Counter(T prev, T next) : _prev_expected(prev), _next_expected(next) {}

  void add(const T &val) {
    _data[val]++;
  }

  auto operator[](const T &key) -> int & {
    return _data[key];
  }

  auto get(const T &key) const -> int {
    auto it = _data.find(key);
    if (it != _data.end()) return it->second;
    return 0;
  }

  [[nodiscard]] auto size() const -> int { return _data.size(); }

  auto down() const -> T {
    return _data.empty() ? T{} : _data.begin()->first;
  }

  auto up() const -> T {
    return _data.empty() ? T{} : _data.rbegin()->first;
  }

  auto count() const -> int {
    int total = 0;
    for (auto const& [val, freq] : _data) total += freq;
    return total;
  }

  auto down_count() const -> int { return get(_prev_expected); }
  auto up_count() const -> int { return get(_next_expected); }

  auto down_expected() const -> T { return _prev_expected; }
  auto up_expected() const -> T { return _next_expected; }

  const std::map<T, int>& data() const { return _data; }
};
}; // namespace prism::tests::helper

#endif // PRISM_TESTS_HELPER_COUNTER_H