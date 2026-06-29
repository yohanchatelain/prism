#include <cmath>
#include <vector>
#include <gtest/gtest.h>
#include "src/prism_api.h"
#include "src/sr_vector.h"
#include "src/utils.h"

namespace vrv = prism::sr::vector::dynamic_dispatch::variable;

TEST(RNModeVectorTest, SpecialValues) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  constexpr size_t count = 5;
  std::vector<float> a_f = {NAN, 1.0f, INFINITY, 0.0f, -0.0f};
  std::vector<float> b_f = {1.0f, NAN, 1.0f, 0.0f, -0.0f};
  std::vector<float> res_f(count);

  vrv::addf32(a_f.data(), b_f.data(), res_f.data(), count);

  EXPECT_TRUE(std::isnan(res_f[0]));
  EXPECT_TRUE(std::isnan(res_f[1]));
  EXPECT_TRUE(std::isinf(res_f[2]));
  EXPECT_EQ(res_f[3], 0.0f);
  EXPECT_EQ(res_f[4], -0.0f);

  std::vector<double> a_d = {NAN, 1.0, INFINITY, 0.0, -0.0};
  std::vector<double> b_d = {1.0, NAN, 1.0, 0.0, -0.0};
  std::vector<double> res_d(count);

  vrv::addf64(a_d.data(), b_d.data(), res_d.data(), count);

  EXPECT_TRUE(std::isnan(res_d[0]));
  EXPECT_TRUE(std::isnan(res_d[1]));
  EXPECT_TRUE(std::isinf(res_d[2]));
  EXPECT_EQ(res_d[3], 0.0);
  EXPECT_EQ(res_d[4], -0.0);

  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeVectorTest, NormalValuesPrecision) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  // Test float with virtual precision t = 10
  prism::sr::set_virtual_precision<float>(10);
  float ulp_f = 1.0f / 512.0f;

  constexpr size_t count = 3;
  std::vector<float> a_f(count, 1.0f);
  std::vector<float> b_f = {0.3f * ulp_f, 0.7f * ulp_f, 0.5f * ulp_f};
  std::vector<float> res_f(count);

  vrv::addf32(a_f.data(), b_f.data(), res_f.data(), count);

  // 1.0 + 0.3 * ulp -> 1.0
  EXPECT_EQ(res_f[0], 1.0f);
  // 1.0 + 0.7 * ulp -> 1.0 + ulp
  EXPECT_EQ(res_f[1], 1.0f + ulp_f);
  // 1.0 + 0.5 * ulp is a tie -> rounds away from zero (to 1.0 + ulp)
  EXPECT_EQ(res_f[2], 1.0f + ulp_f);

  // Test double with virtual precision t = 15
  prism::sr::set_virtual_precision<double>(15);
  double ulp_d = 1.0 / 16384.0;

  std::vector<double> a_d(count, 1.0);
  std::vector<double> b_d = {0.3 * ulp_d, 0.7 * ulp_d, 0.5 * ulp_d};
  std::vector<double> res_d(count);

  vrv::addf64(a_d.data(), b_d.data(), res_d.data(), count);

  // 1.0 + 0.3 * ulp -> 1.0
  EXPECT_EQ(res_d[0], 1.0);
  // 1.0 + 0.7 * ulp -> 1.0 + ulp
  EXPECT_EQ(res_d[1], 1.0 + ulp_d);
  // 1.0 + 0.5 * ulp is a tie -> rounds away from zero (to 1.0 + ulp)
  EXPECT_EQ(res_d[2], 1.0 + ulp_d);

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeVectorTest, TiesRoundingAwayFromZero) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  constexpr size_t count = 2;

  // Test float with different virtual precisions
  for (int t = 5; t <= 20; ++t) {
    prism::sr::set_virtual_precision<float>(t);
    float ulp_f = std::pow(2.0f, -(t - 1));

    std::vector<float> a_f = {1.0f, -1.0f};
    std::vector<float> b_f = {0.5f * ulp_f, -0.5f * ulp_f};
    std::vector<float> res_f(count);

    vrv::addf32(a_f.data(), b_f.data(), res_f.data(), count);

    // Positive tie: rounds away from zero (up)
    EXPECT_EQ(res_f[0], 1.0f + ulp_f);
    // Negative tie: rounds away from zero (down)
    EXPECT_EQ(res_f[1], -1.0f - ulp_f);
  }

  // Test double with different virtual precisions
  for (int t = 5; t <= 40; ++t) {
    prism::sr::set_virtual_precision<double>(t);
    double ulp_d = std::pow(2.0, -(t - 1));

    std::vector<double> a_d = {1.0, -1.0};
    std::vector<double> b_d = {0.5 * ulp_d, -0.5 * ulp_d};
    std::vector<double> res_d(count);

    vrv::addf64(a_d.data(), b_d.data(), res_d.data(), count);

    // Positive tie: rounds away from zero (up)
    EXPECT_EQ(res_d[0], 1.0 + ulp_d);
    // Negative tie: rounds away from zero (down)
    EXPECT_EQ(res_d[1], -1.0 - ulp_d);
  }

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeVectorTest, SubnormalValues) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  constexpr size_t count = 4;

  // Float virtual subnormals at t = 10
  prism::sr::set_virtual_precision<float>(10);
  float ulp_sub_f = std::pow(2.0f, -135.0f);

  std::vector<float> a_f = {ulp_sub_f, -ulp_sub_f, ulp_sub_f, ulp_sub_f};
  std::vector<float> b_f = {0.5f * ulp_sub_f, -0.5f * ulp_sub_f, 0.3f * ulp_sub_f, 0.7f * ulp_sub_f};
  std::vector<float> res_f(count);

  vrv::addf32(a_f.data(), b_f.data(), res_f.data(), count);

  EXPECT_EQ(res_f[0], 2.0f * ulp_sub_f);
  EXPECT_EQ(res_f[1], -2.0f * ulp_sub_f);
  EXPECT_EQ(res_f[2], ulp_sub_f);
  EXPECT_EQ(res_f[3], 2.0f * ulp_sub_f);

  // Double virtual subnormals at t = 20
  prism::sr::set_virtual_precision<double>(20);
  double ulp_sub_d = std::pow(2.0, -1041.0);

  std::vector<double> a_d = {ulp_sub_d, -ulp_sub_d, ulp_sub_d, ulp_sub_d};
  std::vector<double> b_d = {0.5 * ulp_sub_d, -0.5 * ulp_sub_d, 0.3 * ulp_sub_d, 0.7 * ulp_sub_d};
  std::vector<double> res_d(count);

  vrv::addf64(a_d.data(), b_d.data(), res_d.data(), count);

  EXPECT_EQ(res_d[0], 2.0 * ulp_sub_d);
  EXPECT_EQ(res_d[1], -2.0 * ulp_sub_d);
  EXPECT_EQ(res_d[2], ulp_sub_d);
  EXPECT_EQ(res_d[3], 2.0 * ulp_sub_d);

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}
