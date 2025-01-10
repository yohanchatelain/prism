#ifndef __PRISM_TESTS_HELPER_BINOMIAL_TEST_H__
#define __PRISM_TESTS_HELPER_BINOMIAL_TEST_H__

#include <boost/math/distributions/binomial.hpp>

namespace prism::tests::helper {

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

}; // namespace prism::tests::helper

#endif // __PRISM_TESTS_HELPER_BINOMIAL_TEST_H__
