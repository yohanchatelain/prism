#include <cmath>
#include <limits>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_array.h"

namespace {

TEST(ArraySpecialValuesTest, FloatSpecialValues) {
  constexpr float inf = std::numeric_limits<float>::infinity();
  constexpr float nan = std::numeric_limits<float>::quiet_NaN();
  constexpr float subnormal = 1e-40f;

  std::vector<float> a = {0.0f, -0.0f, inf, -inf, nan, subnormal, 1.0f, -1.0f};
  std::vector<float> b = {1.0f,  2.0f, 1.0f, 1.0f, 1.0f, subnormal, inf, nan};
  std::vector<float> res(a.size(), 0.0f);

  // Add
  prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), a.size());
  EXPECT_EQ(res[0], 1.0f);
  EXPECT_EQ(res[1], 2.0f);
  EXPECT_TRUE(std::isinf(res[2]) && res[2] > 0);
  EXPECT_TRUE(std::isinf(res[3]) && res[3] < 0);
  EXPECT_TRUE(std::isnan(res[4]));
  EXPECT_GT(res[5], 0.0f);
  EXPECT_TRUE(std::isinf(res[6]) && res[6] > 0);
  EXPECT_TRUE(std::isnan(res[7]));

  // Mul
  prism::sr::array::PRISM_DISPATCH::mulf32(a.data(), b.data(), res.data(), a.size());
  EXPECT_EQ(res[0], 0.0f);
  EXPECT_TRUE(std::isinf(res[2]) && res[2] > 0);
  EXPECT_TRUE(std::isinf(res[3]) && res[3] < 0);
  EXPECT_TRUE(std::isnan(res[4]));
  EXPECT_TRUE(std::isinf(res[6]) && res[6] > 0);
  EXPECT_TRUE(std::isnan(res[7]));
}

TEST(ArraySpecialValuesTest, DoubleSpecialValues) {
  constexpr double inf = std::numeric_limits<double>::infinity();
  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  constexpr double subnormal = 1e-315;

  std::vector<double> a = {0.0, -0.0, inf, -inf, nan, subnormal, 1.0, -1.0};
  std::vector<double> b = {1.0,  2.0, 1.0, 1.0, 1.0, subnormal, inf, nan};
  std::vector<double> res(a.size(), 0.0);

  // Add
  prism::sr::array::PRISM_DISPATCH::addf64(a.data(), b.data(), res.data(), a.size());
  EXPECT_EQ(res[0], 1.0);
  EXPECT_EQ(res[1], 2.0);
  EXPECT_TRUE(std::isinf(res[2]) && res[2] > 0);
  EXPECT_TRUE(std::isinf(res[3]) && res[3] < 0);
  EXPECT_TRUE(std::isnan(res[4]));
  EXPECT_GT(res[5], 0.0);
  EXPECT_TRUE(std::isinf(res[6]) && res[6] > 0);
  EXPECT_TRUE(std::isnan(res[7]));

  // Mul
  prism::sr::array::PRISM_DISPATCH::mulf64(a.data(), b.data(), res.data(), a.size());
  EXPECT_EQ(res[0], 0.0);
  EXPECT_TRUE(std::isinf(res[2]) && res[2] > 0);
  EXPECT_TRUE(std::isinf(res[3]) && res[3] < 0);
  EXPECT_TRUE(std::isnan(res[4]));
  EXPECT_TRUE(std::isinf(res[6]) && res[6] > 0);
  EXPECT_TRUE(std::isnan(res[7]));
}

} // namespace
