#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_api.h"

namespace {

TEST(ArrayCAPITest, FloatOperations) {
  const size_t N = 33;
  std::vector<float> a(N, 2.0f);
  std::vector<float> b(N, 3.0f);
  std::vector<float> c(N, 1.0f);
  std::vector<float> res(N, 0.0f);

  // SR Add
  interflop_prism_sr_add_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 5.0f, 1e-5f);

  // SR Sub
  interflop_prism_sr_sub_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], -1.0f, 1e-5f);

  // SR Mul
  interflop_prism_sr_mul_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 6.0f, 1e-5f);

  // SR Div
  interflop_prism_sr_div_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 2.0f / 3.0f, 1e-5f);

  // SR Sqrt
  interflop_prism_sr_sqrt_f32(a.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], std::sqrt(2.0f), 1e-5f);

  // SR FMA
  interflop_prism_sr_fma_f32(a.data(), b.data(), c.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 7.0f, 1e-5f);

  // UD Add
  interflop_prism_ud_add_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 5.0f, 1e-5f);

  // UD Sub
  interflop_prism_ud_sub_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], -1.0f, 1e-5f);

  // UD Mul
  interflop_prism_ud_mul_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 6.0f, 1e-5f);

  // UD Div
  interflop_prism_ud_div_f32(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 2.0f / 3.0f, 1e-5f);

  // UD Sqrt
  interflop_prism_ud_sqrt_f32(a.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], std::sqrt(2.0f), 1e-5f);

  // UD FMA
  interflop_prism_ud_fma_f32(a.data(), b.data(), c.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 7.0f, 1e-5f);
}

TEST(ArrayCAPITest, DoubleOperations) {
  const size_t N = 33;
  std::vector<double> a(N, 2.0);
  std::vector<double> b(N, 3.0);
  std::vector<double> c(N, 1.0);
  std::vector<double> res(N, 0.0);

  // SR Add
  interflop_prism_sr_add_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 5.0, 1e-12);

  // SR Sub
  interflop_prism_sr_sub_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], -1.0, 1e-12);

  // SR Mul
  interflop_prism_sr_mul_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 6.0, 1e-12);

  // SR Div
  interflop_prism_sr_div_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 2.0 / 3.0, 1e-12);

  // SR Sqrt
  interflop_prism_sr_sqrt_f64(a.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], std::sqrt(2.0), 1e-12);

  // SR FMA
  interflop_prism_sr_fma_f64(a.data(), b.data(), c.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 7.0, 1e-12);

  // UD Add
  interflop_prism_ud_add_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 5.0, 1e-12);

  // UD Sub
  interflop_prism_ud_sub_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], -1.0, 1e-12);

  // UD Mul
  interflop_prism_ud_mul_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 6.0, 1e-12);

  // UD Div
  interflop_prism_ud_div_f64(a.data(), b.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 2.0 / 3.0, 1e-12);

  // UD Sqrt
  interflop_prism_ud_sqrt_f64(a.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], std::sqrt(2.0), 1e-12);

  // UD FMA
  interflop_prism_ud_fma_f64(a.data(), b.data(), c.data(), res.data(), N);
  for (size_t i = 0; i < N; ++i) EXPECT_NEAR(res[i], 7.0, 1e-12);
}

} // namespace
