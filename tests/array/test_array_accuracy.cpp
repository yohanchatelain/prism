#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_array.h"
#include "tests/helper/binomial_test.h"

namespace {

TEST(ArrayAccuracyTest, SRBinomialDistribution) {
  constexpr size_t array_len = 16;
  constexpr size_t repetitions = 10000;
  constexpr double p_expected = 0.5; // for a = 1.0, b = 0.5 * 2^(-23) = 2^(-24)
  constexpr float ulp = 0x1.0p-23f;
  constexpr float half_ulp = 0x1.0p-24f;

  std::vector<float> a(array_len, 1.0f);
  std::vector<float> b(array_len, half_ulp);
  std::vector<float> res(array_len, 0.0f);
  std::vector<size_t> round_up_counts(array_len, 0);

  for (size_t r = 0; r < repetitions; ++r) {
    prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), array_len);
    for (size_t i = 0; i < array_len; ++i) {
      if (res[i] == 1.0f + ulp) {
        round_up_counts[i]++;
      } else {
        ASSERT_EQ(res[i], 1.0f);
      }
    }
  }

  // Binomial test for each lane with Bonferroni correction
  const double alpha = 0.001 / array_len;
  for (size_t i = 0; i < array_len; ++i) {
    auto test = prism::tests::helper::binomial_test(repetitions, round_up_counts[i], p_expected);
    EXPECT_GE(test.pvalue, alpha)
        << "Lane " << i << " failed statistical binomial test: #up="
        << round_up_counts[i] << "/" << repetitions << ", pvalue=" << test.pvalue;
  }
}

TEST(ArrayAccuracyTest, UDBinomialDistribution) {
  constexpr size_t array_len = 16;
  constexpr size_t repetitions = 10000;
  constexpr double p_expected = 0.5;

  std::vector<float> a(array_len, 1.0f);
  std::vector<float> b(array_len, 2.0f);
  std::vector<float> res(array_len, 0.0f);
  std::vector<size_t> round_up_counts(array_len, 0);

  for (size_t r = 0; r < repetitions; ++r) {
    prism::ud::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), array_len);
    for (size_t i = 0; i < array_len; ++i) {
      // 1.0 + 2.0 = 3.0. In UD mode, result is either 3.0 - 1ulp or 3.0 + 1ulp with equal probability 0.5
      if (res[i] > 3.0f) {
        round_up_counts[i]++;
      }
    }
  }

  const double alpha = 0.001 / array_len;
  for (size_t i = 0; i < array_len; ++i) {
    auto test = prism::tests::helper::binomial_test(repetitions, round_up_counts[i], p_expected);
    EXPECT_GE(test.pvalue, alpha)
        << "Lane " << i << " failed UD statistical binomial test: #up="
        << round_up_counts[i] << "/" << repetitions << ", pvalue=" << test.pvalue;
  }
}

} // namespace
