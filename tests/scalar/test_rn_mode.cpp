#include <cmath>
#include <gtest/gtest.h>
#include "src/prism_api.h"
#include "src/sr_scalar.h"
#include "src/utils.h"

namespace srd = prism::sr::scalar::dynamic_dispatch;

TEST(RNModeTest, BasicGetSet) {
  // Test roundtrip of the rounding mode C API
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
  EXPECT_EQ(interflop_prism_get_rounding_mode(), INTERFLOP_PRISM_SR);

  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);
  EXPECT_EQ(interflop_prism_get_rounding_mode(), INTERFLOP_PRISM_RN);

  // Revert to SR
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeTest, SpecialValues) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  // Special values on float
  EXPECT_TRUE(std::isnan(srd::addf32(NAN, 1.0f)));
  EXPECT_TRUE(std::isnan(srd::addf32(1.0f, NAN)));
  EXPECT_TRUE(std::isinf(srd::addf32(INFINITY, 1.0f)));
  EXPECT_EQ(srd::addf32(0.0f, 0.0f), 0.0f);
  EXPECT_EQ(srd::addf32(-0.0f, -0.0f), -0.0f);

  // Special values on double
  EXPECT_TRUE(std::isnan(srd::addf64(NAN, 1.0)));
  EXPECT_TRUE(std::isnan(srd::addf64(1.0, NAN)));
  EXPECT_TRUE(std::isinf(srd::addf64(INFINITY, 1.0)));
  EXPECT_EQ(srd::addf64(0.0, 0.0), 0.0);
  EXPECT_EQ(srd::addf64(-0.0, -0.0), -0.0);

  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeTest, NormalValuesPrecision) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  // Test float with virtual precision t = 10
  prism::sr::set_virtual_precision<float>(10);
  // ulp_t for 1.0f is 2^(-9) = 1.0f / 512.0f
  float ulp_f = 1.0f / 512.0f;

  // 1.0 + 0.3 * ulp should round to 1.0
  EXPECT_EQ(srd::addf32(1.0f, 0.3f * ulp_f), 1.0f);
  // 1.0 + 0.7 * ulp should round to 1.0 + ulp
  EXPECT_EQ(srd::addf32(1.0f, 0.7f * ulp_f), 1.0f + ulp_f);
  // 1.0 + 0.5 * ulp is a tie -> round away from zero (to 1.0 + ulp)
  EXPECT_EQ(srd::addf32(1.0f, 0.5f * ulp_f), 1.0f + ulp_f);

  // Test double with virtual precision t = 15
  prism::sr::set_virtual_precision<double>(15);
  // ulp_t for 1.0 is 2^(-14) = 1.0 / 16384.0
  double ulp_d = 1.0 / 16384.0;

  // 1.0 + 0.3 * ulp should round to 1.0
  EXPECT_EQ(srd::addf64(1.0, 0.3 * ulp_d), 1.0);
  // 1.0 + 0.7 * ulp should round to 1.0 + ulp
  EXPECT_EQ(srd::addf64(1.0, 0.7 * ulp_d), 1.0 + ulp_d);
  // 1.0 + 0.5 * ulp is a tie -> round away from zero (to 1.0 + ulp)
  EXPECT_EQ(srd::addf64(1.0, 0.5 * ulp_d), 1.0 + ulp_d);

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeTest, TiesRoundingAwayFromZero) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  // Test both float and double with different virtual precisions
  for (int t = 5; t <= 20; ++t) {
    prism::sr::set_virtual_precision<float>(t);
    float ulp_f = std::pow(2.0f, -(t - 1));

    // Positive tie: rounds away from zero (up)
    EXPECT_EQ(srd::addf32(1.0f, 0.5f * ulp_f), 1.0f + ulp_f);
    // Negative tie: rounds away from zero (down/larger magnitude)
    EXPECT_EQ(srd::addf32(-1.0f, -0.5f * ulp_f), -1.0f - ulp_f);
  }

  for (int t = 5; t <= 40; ++t) {
    prism::sr::set_virtual_precision<double>(t);
    double ulp_d = std::pow(2.0, -(t - 1));

    // Positive tie: rounds away from zero (up)
    EXPECT_EQ(srd::addf64(1.0, 0.5 * ulp_d), 1.0 + ulp_d);
    // Negative tie: rounds away from zero (down/larger magnitude)
    EXPECT_EQ(srd::addf64(-1.0, -0.5 * ulp_d), -1.0 - ulp_d);
  }

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}

TEST(RNModeTest, SubnormalValues) {
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_RN);

  // For float virtual subnormals at t = 10
  prism::sr::set_virtual_precision<float>(10);
  // virtual subnormal ulp at t=10 is 2^(-126 - 9) = 2^(-135)
  float ulp_sub_f = std::pow(2.0f, -135.0f);

  // Tie on virtual subnormal: rounds away from zero
  EXPECT_EQ(srd::addf32(ulp_sub_f, 0.5f * ulp_sub_f), 2.0f * ulp_sub_f);
  EXPECT_EQ(srd::addf32(-ulp_sub_f, -0.5f * ulp_sub_f), -2.0f * ulp_sub_f);

  // Normal rounding for virtual subnormals
  EXPECT_EQ(srd::addf32(ulp_sub_f, 0.3f * ulp_sub_f), ulp_sub_f);
  EXPECT_EQ(srd::addf32(ulp_sub_f, 0.7f * ulp_sub_f), 2.0f * ulp_sub_f);

  // For double virtual subnormals at t = 20
  prism::sr::set_virtual_precision<double>(20);
  // virtual subnormal ulp at t=20 is 2^(-1022 - 19) = 2^(-1041)
  double ulp_sub_d = std::pow(2.0, -1041.0);

  // Tie on virtual subnormal: rounds away from zero
  EXPECT_EQ(srd::addf64(ulp_sub_d, 0.5 * ulp_sub_d), 2.0 * ulp_sub_d);
  EXPECT_EQ(srd::addf64(-ulp_sub_d, -0.5 * ulp_sub_d), -2.0 * ulp_sub_d);

  // Normal rounding for virtual subnormals
  EXPECT_EQ(srd::addf64(ulp_sub_d, 0.3 * ulp_sub_d), ulp_sub_d);
  EXPECT_EQ(srd::addf64(ulp_sub_d, 0.7 * ulp_sub_d), 2.0 * ulp_sub_d);

  // Clean up
  prism::sr::set_virtual_precision<float>(24);
  prism::sr::set_virtual_precision<double>(53);
  interflop_prism_set_rounding_mode(INTERFLOP_PRISM_SR);
}
