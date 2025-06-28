#include <cmath>
#include <vector>
#include <memory>
#include <chrono>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "hwy/tests/hwy_gtest.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector.h"
#include "src/ud_vector.h"

#include "tests/performance/benchmark_suite.h"

namespace prism::performance_test {

namespace sr = prism::sr::vector::PRISM_DISPATCH;
namespace ud = prism::ud::vector::PRISM_DISPATCH;
namespace perf = prism::tests::performance;

// Test data sizes from small cache-friendly to large memory-bound
const std::vector<size_t> TEST_SIZES = {
  1024,    // L1 cache friendly
  4096,    // L2 cache friendly  
  16384,   // L3 cache friendly
  65536,   // Memory bound small
  262144,  // Memory bound medium
  1048576  // Memory bound large
};

// Performance test class
class PerformanceRegressionTest : public ::testing::Test {
protected:
  perf::BenchmarkSuite benchmark_suite_;
  
  void SetUp() override {
    // Configure benchmark parameters
    perf::BenchmarkConfig config;
    config.warmup_iterations = 1000;
    config.measurement_iterations = 5000; // Reduced for CI/regression testing
    config.test_sizes = TEST_SIZES;
    config.use_cpu_cycles = true;
    config.remove_outliers = true;
    
    benchmark_suite_ = perf::BenchmarkSuite(config);
  }
  
  void TearDown() override {
    // Save results and print summary
    benchmark_suite_.save_results("benchmark_results");
    benchmark_suite_.print_summary();
  }
};

// Benchmark wrappers for different operations
namespace {

// SR (Stochastic Rounding) Operations
void benchmark_sr_add_f32(float* a, float* b, float* result, size_t count) {
  sr::variable::addf32(a, b, result, count);
}

void benchmark_sr_add_f64(double* a, double* b, double* result, size_t count) {
  sr::variable::addf64(a, b, result, count);
}

void benchmark_sr_mul_f32(float* a, float* b, float* result, size_t count) {
  sr::variable::mulf32(a, b, result, count);
}

void benchmark_sr_mul_f64(double* a, double* b, double* result, size_t count) {
  sr::variable::mulf64(a, b, result, count);
}

void benchmark_sr_fma_f32(float* a, float* b, float* c, size_t count) {
  sr::variable::fmaf32(a, b, c, c, count); // Use c as both input and output
}

void benchmark_sr_fma_f64(double* a, double* b, double* c, size_t count) {
  sr::variable::fmaf64(a, b, c, c, count); // Use c as both input and output
}

// UD (Uniform Distribution) Operations  
void benchmark_ud_add_f32(float* a, float* b, float* result, size_t count) {
  ud::variable::addf32(a, b, result, count);
}

void benchmark_ud_add_f64(double* a, double* b, double* result, size_t count) {
  ud::variable::addf64(a, b, result, count);
}

void benchmark_ud_mul_f32(float* a, float* b, float* result, size_t count) {
  ud::variable::mulf32(a, b, result, count);
}

void benchmark_ud_mul_f64(double* a, double* b, double* result, size_t count) {
  ud::variable::mulf64(a, b, result, count);
}

// Standard floating-point operations for baseline comparison
void benchmark_std_add_f32(float* a, float* b, float* result, size_t count) {
  for (size_t i = 0; i < count; i++) {
    result[i] = a[i] + b[i];
  }
}

void benchmark_std_add_f64(double* a, double* b, double* result, size_t count) {
  for (size_t i = 0; i < count; i++) {
    result[i] = a[i] + b[i];
  }
}

void benchmark_std_mul_f32(float* a, float* b, float* result, size_t count) {
  for (size_t i = 0; i < count; i++) {
    result[i] = a[i] * b[i];
  }
}

void benchmark_std_mul_f64(double* a, double* b, double* result, size_t count) {
  for (size_t i = 0; i < count; i++) {
    result[i] = a[i] * b[i];
  }
}

void benchmark_std_fma_f32(float* a, float* b, float* c, size_t count) {
  for (size_t i = 0; i < count; i++) {
    c[i] = a[i] * b[i] + c[i];
  }
}

void benchmark_std_fma_f64(double* a, double* b, double* c, size_t count) {
  for (size_t i = 0; i < count; i++) {
    c[i] = a[i] * b[i] + c[i];
  }
}

} // anonymous namespace

// Performance regression tests

TEST_F(PerformanceRegressionTest, SR_Operations_Float32) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_sr_add_f32), float>("SR_Add", benchmark_sr_add_f32);
  benchmark_suite_.benchmark_operation<decltype(benchmark_sr_mul_f32), float>("SR_Mul", benchmark_sr_mul_f32);
}

TEST_F(PerformanceRegressionTest, SR_Operations_Float64) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_sr_add_f64), double>("SR_Add", benchmark_sr_add_f64);
  benchmark_suite_.benchmark_operation<decltype(benchmark_sr_mul_f64), double>("SR_Mul", benchmark_sr_mul_f64);
}

TEST_F(PerformanceRegressionTest, UD_Operations_Float32) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_ud_add_f32), float>("UD_Add", benchmark_ud_add_f32);
  benchmark_suite_.benchmark_operation<decltype(benchmark_ud_mul_f32), float>("UD_Mul", benchmark_ud_mul_f32);
}

TEST_F(PerformanceRegressionTest, UD_Operations_Float64) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_ud_add_f64), double>("UD_Add", benchmark_ud_add_f64);
  benchmark_suite_.benchmark_operation<decltype(benchmark_ud_mul_f64), double>("UD_Mul", benchmark_ud_mul_f64);
}

TEST_F(PerformanceRegressionTest, Baseline_Operations_Float32) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_std_add_f32), float>("STD_Add", benchmark_std_add_f32);
  benchmark_suite_.benchmark_operation<decltype(benchmark_std_mul_f32), float>("STD_Mul", benchmark_std_mul_f32);
}

TEST_F(PerformanceRegressionTest, Baseline_Operations_Float64) {
  benchmark_suite_.benchmark_operation<decltype(benchmark_std_add_f64), double>("STD_Add", benchmark_std_add_f64);
  benchmark_suite_.benchmark_operation<decltype(benchmark_std_mul_f64), double>("STD_Mul", benchmark_std_mul_f64);
}

void benchmark_memory_copy_f32(float* a, float* b, float* result, size_t count) {
  std::memcpy(result, a, count * sizeof(float));
}

// Memory bandwidth tests
TEST_F(PerformanceRegressionTest, Memory_Bandwidth_Tests) {
  // Test pure memory copy performance
  benchmark_suite_.benchmark_operation<decltype(benchmark_memory_copy_f32), float>("Memory_Copy", benchmark_memory_copy_f32);
  
  // Test memory throughput with simple operations - same as baseline
  benchmark_suite_.benchmark_operation<decltype(benchmark_std_add_f32), float>("Memory_Bandwidth", benchmark_std_add_f32);
}

void benchmark_random_access_f32(float* a, float* b, float* result, size_t count) {
  // Use simple LCG for reproducible "random" pattern
  uint32_t seed = 12345;
  for (size_t i = 0; i < count; i++) {
    seed = seed * 1664525 + 1013904223;
    size_t idx = seed % count;
    result[i] = a[idx] + b[idx];
  }
}

// Cache performance tests
TEST_F(PerformanceRegressionTest, Cache_Performance_Tests) {
  // Sequential access pattern - use SR operations
  benchmark_suite_.benchmark_operation<decltype(benchmark_sr_add_f32), float>("Sequential_Access", benchmark_sr_add_f32);
  
  // Random access pattern (cache unfriendly)
  benchmark_suite_.benchmark_operation<decltype(benchmark_random_access_f32), float>("Random_Access", benchmark_random_access_f32);
}

} // namespace prism::performance_test