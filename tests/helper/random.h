#ifndef __PRISM_TESTS_HELPER_RANDOM_H__
#define __PRISM_TESTS_HELPER_RANDOM_H__

#include <random>

namespace prism::tests::helper {

struct RNG {
private:
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

public:
  explicit RNG(double a = 0.0, double b = 1.0) : gen(RNG::rd()), dis(a, b){};
  auto operator()() -> double { return dis(gen); }
};

}; // namespace prism::tests::helper

#endif // __PRISM_TESTS_HELPER_RANDOM_H__