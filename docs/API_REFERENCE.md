# PRISM API Reference

Complete API documentation for the PRISM probabilistic rounding library.

## 📖 Table of Contents

- [Overview](#overview)
- [Stochastic Rounding API](#stochastic-rounding-api)
- [Up-Down Rounding API](#up-down-rounding-api)
- [Configuration Options](#configuration-options)
- [Performance Testing API](#performance-testing-api)
- [Utility Functions](#utility-functions)
- [Error Handling](#error-handling)

---

## Overview

PRISM provides probabilistic rounding operations through multiple interfaces:

### Namespace Structure
```cpp
prism::sr::vector::PRISM_DISPATCH     // Stochastic Rounding
prism::ud::vector::PRISM_DISPATCH     // Up-Down Rounding
prism::tests::performance             // Performance Testing Framework
```

### Dispatch Modes
- **`dynamic_dispatch`**: Runtime architecture selection (default)
- **`static_dispatch`**: Compile-time architecture optimization

---

## Stochastic Rounding API

**Header**: `#include "src/sr_vector.h"`  
**Namespace**: `prism::sr::vector::PRISM_DISPATCH`

### Array Interface (`variable` namespace)

#### Float32 Operations
```cpp
void addf32(const float* a, const float* b, float* result, size_t count);
```
**Description**: Element-wise addition with stochastic rounding  
**Parameters**:
- `a`: Input array A (aligned to 32-byte boundary recommended)
- `b`: Input array B (aligned to 32-byte boundary recommended)  
- `result`: Output array (aligned to 32-byte boundary recommended)
- `count`: Number of elements to process

**Example**:
```cpp
float a[1000], b[1000], result[1000];
// ... initialize data ...
variable::addf32(a, b, result, 1000);
```

```cpp
void subf32(const float* a, const float* b, float* result, size_t count);
```
**Description**: Element-wise subtraction with stochastic rounding

```cpp
void mulf32(const float* a, const float* b, float* result, size_t count);
```
**Description**: Element-wise multiplication with stochastic rounding

```cpp
void divf32(const float* a, const float* b, float* result, size_t count);
```
**Description**: Element-wise division with stochastic rounding

```cpp
void sqrtf32(const float* a, float* result, size_t count);
```
**Description**: Element-wise square root with stochastic rounding  
**Parameters**:
- `a`: Input array (must contain non-negative values)
- `result`: Output array
- `count`: Number of elements to process

```cpp
void fmaf32(const float* a, const float* b, const float* c, float* result, size_t count);
```
**Description**: Fused multiply-add (a * b + c) with stochastic rounding  
**Parameters**:
- `a`: First multiplicand array
- `b`: Second multiplicand array  
- `c`: Addend array
- `result`: Output array (result = a * b + c)
- `count`: Number of elements to process

#### Float64 Operations
```cpp
void addf64(const double* a, const double* b, double* result, size_t count);
void subf64(const double* a, const double* b, double* result, size_t count);
void mulf64(const double* a, const double* b, double* result, size_t count);
void divf64(const double* a, const double* b, double* result, size_t count);
void sqrtf64(const double* a, double* result, size_t count);
void fmaf64(const double* a, const double* b, const double* c, double* result, size_t count);
```
**Description**: Double-precision equivalents of float32 operations

### Fixed-Size Vector Interface (`fixed` namespace)

#### Vector Type Definitions
```cpp
typedef float f32x2_v __attribute__((vector_size(8)));      // 2x float32
typedef float f32x4_v __attribute__((vector_size(16)));     // 4x float32  
typedef float f32x8_v __attribute__((vector_size(32)));     // 8x float32
typedef float f32x16_v __attribute__((vector_size(64)));    // 16x float32

typedef double f64x2_v __attribute__((vector_size(16)));    // 2x float64
typedef double f64x4_v __attribute__((vector_size(32)));    // 4x float64
typedef double f64x8_v __attribute__((vector_size(64)));    // 8x float64
typedef double f64x16_v __attribute__((vector_size(128)));  // 16x float64
```

#### 128-bit Vector Operations (SSE4+)
```cpp
#if HWY_MAX_BYTES >= 16
auto addf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto subf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto mulf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto divf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto sqrtf32x4(f32x4_v a) -> f32x4_v;
auto fmaf32x4(f32x4_v a, f32x4_v b, f32x4_v c) -> f32x4_v;

auto addf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto subf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto mulf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto divf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto sqrtf64x2(f64x2_v a) -> f64x2_v;
auto fmaf64x2(f64x2_v a, f64x2_v b, f64x2_v c) -> f64x2_v;
#endif
```

#### 256-bit Vector Operations (AVX2+)
```cpp
#if HWY_MAX_BYTES >= 32
auto addf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto subf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto mulf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto divf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto sqrtf32x8(f32x8_v a) -> f32x8_v;
auto fmaf32x8(f32x8_v a, f32x8_v b, f32x8_v c) -> f32x8_v;

auto addf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto subf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto mulf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto divf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto sqrtf64x4(f64x4_v a) -> f64x4_v;
auto fmaf64x4(f64x4_v a, f64x4_v b, f64x4_v c) -> f64x4_v;
#endif
```

#### 512-bit Vector Operations (AVX-512+)
```cpp
#if HWY_MAX_BYTES >= 64
auto addf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto subf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto mulf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto divf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto sqrtf32x16(f32x16_v a) -> f32x16_v;
auto fmaf32x16(f32x16_v a, f32x16_v b, f32x16_v c) -> f32x16_v;

auto addf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto subf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto mulf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto divf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto sqrtf64x8(f64x8_v a) -> f64x8_v;
auto fmaf64x8(f64x8_v a, f64x8_v b, f64x8_v c) -> f64x8_v;
#endif
```

**Example**:
```cpp
#include "src/sr_vector.h"
using namespace prism::sr::vector::PRISM_DISPATCH;

// Check vector width at compile time
#if HWY_MAX_BYTES >= 32
    f32x8_v a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    f32x8_v b = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    f32x8_v result = fixed::addf32x8(a, b);
#endif
```

---

## Up-Down Rounding API

**Header**: `#include "src/ud_vector.h"`  
**Namespace**: `prism::ud::vector::PRISM_DISPATCH`

### Array Interface (`variable` namespace)

The Up-Down rounding API mirrors the Stochastic Rounding API but implements uniform probability rounding (+/-1 ULP with equal probability).

#### Float32 Operations
```cpp
void addf32(const float* a, const float* b, float* result, size_t count);
void subf32(const float* a, const float* b, float* result, size_t count);
void mulf32(const float* a, const float* b, float* result, size_t count);
void divf32(const float* a, const float* b, float* result, size_t count);
void sqrtf32(const float* a, float* result, size_t count);
void fmaf32(const float* a, const float* b, const float* c, float* result, size_t count);
```

#### Float64 Operations
```cpp
void addf64(const double* a, const double* b, double* result, size_t count);
void subf64(const double* a, const double* b, double* result, size_t count);
void mulf64(const double* a, const double* b, double* result, size_t count);
void divf64(const double* a, const double* b, double* result, size_t count);
void sqrtf64(const double* a, double* result, size_t count);
void fmaf64(const double* a, const double* b, const double* c, double* result, size_t count);
```

### Fixed-Size Vector Interface (`fixed` namespace)

The fixed-size vector interface for Up-Down rounding uses the same function signatures as Stochastic Rounding but with different probabilistic behavior.

**Example**:
```cpp
#include "src/ud_vector.h"
using namespace prism::ud::vector::PRISM_DISPATCH;

float a[1000], b[1000], result[1000];
// ... initialize data ...
variable::addf32(a, b, result, 1000);  // Up-Down rounding
```

---

## Configuration Options

### Compilation Macros

#### Dispatch Mode Selection
```cpp
// Dynamic dispatch (default) - runtime architecture selection
#define PRISM_DISPATCH dynamic_dispatch

// Static dispatch - compile-time optimization  
#define PRISM_DISPATCH static_dispatch
```

#### Random Number Generation
```cpp
// Full-precision random bits (slower, more accurate)
#define PRISM_RANDOM_FULLBITS

// Partial-precision random bits (faster, default)
#undef PRISM_RANDOM_FULLBITS
```

#### Debug Mode
```cpp
// Enable debug output and assertions
#define PRISM_DEBUG

// Enable stochastic rounding debug output
#define SR_DEBUG

// Enable xoshiro random number generator debug
#define PRISM_DEBUG_XOSHIRO
```

### Environment Variables

#### Statistical Testing Configuration
```bash
export PRISM_TEST_ALPHA=0.01              # Significance level (default: 0.000001)
export PRISM_TEST_REPETITIONS=10000       # Test repetitions (default: 10000)
export PRISM_TEST_THRESHOLD=0.000001      # Alpha threshold (alias for ALPHA)
export PRISM_TEST_MAX_RETRIES=3           # Maximum retry attempts for robust testing
export PRISM_TEST_DISABLE_BONFERRONI=0    # Disable Bonferroni correction (0=enabled, 1=disabled)
```

#### Performance Testing Configuration
```bash
export PRISM_TEST_ITERATIONS=50000        # Performance test iterations
export PRISM_TEST_WARMUP=2000             # Warmup iterations
```

---

## Performance Testing API

**Header**: `#include "tests/performance/benchmark_suite.h"`  
**Namespace**: `prism::tests::performance`

### Benchmark Configuration
```cpp
struct BenchmarkConfig {
    size_t warmup_iterations = 1000;
    size_t measurement_iterations = 10000;
    bool use_cpu_cycles = true;
    bool remove_outliers = true;
    double outlier_threshold = 3.0;
    std::vector<size_t> test_sizes = {1024, 4096, 16384, 65536, 262144};
    std::string output_format = "json";
};
```

### Benchmark Statistics
```cpp
struct BenchmarkStats {
    double min_time = 0.0;
    double max_time = 0.0;
    double mean_time = 0.0;
    double median_time = 0.0;
    double stddev_time = 0.0;
    double p95_time = 0.0;
    double p99_time = 0.0;
    size_t iterations = 0;
    size_t elements_processed = 0;
    double throughput_mops = 0.0;
    std::string operation_name;
    std::string data_type;
    size_t vector_size = 0;
};
```

### Benchmark Suite Class
```cpp
class BenchmarkSuite {
public:
    explicit BenchmarkSuite(const BenchmarkConfig& config = {});
    
    template<typename Func, typename T>
    void benchmark_operation(const std::string& operation_name, Func&& func);
    
    template<typename Func, typename T>
    void benchmark_operation_size(const std::string& operation_name, 
                                  Func&& func, size_t size);
    
    void save_results(const std::string& output_dir = "benchmark_results");
    void print_summary() const;
    const BenchmarkResult& get_results() const;
};
```

### High-Precision Timer
```cpp
class HighPrecisionTimer {
public:
    void start();
    double elapsed_nanoseconds() const;
    uint64_t elapsed_cycles() const;
};
```

**Example Usage**:
```cpp
#include "tests/performance/benchmark_suite.h"

void my_benchmark_function(float* a, float* b, float* result, size_t count) {
    prism::sr::vector::PRISM_DISPATCH::variable::addf32(a, b, result, count);
}

int main() {
    prism::tests::performance::BenchmarkSuite suite;
    suite.benchmark_operation<decltype(my_benchmark_function), float>(
        "SR_Add_f32", my_benchmark_function);
    
    suite.save_results();
    suite.print_summary();
    return 0;
}
```

---

## Utility Functions

**Header**: `#include "src/utils.h"`

### IEEE-754 Utilities
```cpp
template<typename T>
struct IEEE754 {
    using H = /* appropriate high-precision type */;
    static constexpr int precision;
    static constexpr int mantissa;
    static constexpr int max_exponent;
    static constexpr int min_exponent;
    static constexpr int min_exponent_subnormal;
    static constexpr T min_subnormal;
    static constexpr T ulp;
    static constexpr T epsilon;
};

// Specializations
IEEE754<float>    // precision=24, mantissa=23, etc.
IEEE754<double>   // precision=53, mantissa=52, etc.
```

### Random Number Generation
**Header**: `#include "src/xoshiro.h"`

```cpp
namespace prism::vector::xoshiro::HWY_NAMESPACE {
    // Generate uniform random vectors
    template<typename T>
    auto uniform() -> /* vector type */;
    
    // Generate random bits
    auto randombit(std::uint64_t u) -> /* vector type */;
}
```

### Debug Utilities
**Header**: `#include "src/debug_vector-inl.h"`

```cpp
namespace prism::vector::HWY_NAMESPACE {
    template<class D, class V>
    void debug_vec(const D d, const std::string& label, const V& vec);
    
    void debug_msg(const std::string& message);
    void debug_reset();
}
```

---

## Error Handling

### Return Values
- Most PRISM functions are `void` and do not return error codes
- Invalid inputs (NaN, infinity) are handled according to IEEE-754 standards
- Memory alignment issues may cause undefined behavior

### Exception Safety
```cpp
// PRISM functions provide basic exception safety:
// - No exceptions thrown from core arithmetic operations
// - Memory allocation in performance testing may throw std::bad_alloc
// - File I/O in benchmarking may throw std::runtime_error
```

### Debugging Support
```cpp
// Enable debug mode for detailed error information
#define PRISM_DEBUG
#include "src/sr_vector.h"

// Debug output will be written to stderr
// Use debug_reset() to clear accumulated debug state
```

### Input Validation
```cpp
// Array interface input requirements:
// - Pointers must not be nullptr (undefined behavior if violated)
// - count must be > 0 (undefined behavior if count == 0)
// - Arrays should not overlap unless result == a or result == b
// - 32-byte alignment recommended for optimal performance

// Vector interface input requirements:  
// - All vector elements must be valid floating-point values
// - NaN and infinity are handled per IEEE-754 specification
// - No additional validation performed
```

### Architecture Requirements
```cpp
// Minimum requirements:
// - x86-64 with SSE4.1 support
// - ARMv8 with NEON support (experimental)

// Feature detection:
#if HWY_MAX_BYTES >= 16
    // 128-bit vectors available
#endif

#if HWY_MAX_BYTES >= 32  
    // 256-bit vectors available
#endif

#if HWY_MAX_BYTES >= 64
    // 512-bit vectors available  
#endif
```

---

## Performance Considerations

### Memory Alignment
```cpp
// Recommended alignment for optimal performance
alignas(32) float data[1000];  // 32-byte alignment for AVX2
alignas(64) double data[500];  // 64-byte alignment for AVX-512
```

### Batch Size Optimization
```cpp
// Optimal batch sizes for different architectures:
// SSE4:   Process in multiples of 4 (float32) or 2 (float64)
// AVX2:   Process in multiples of 8 (float32) or 4 (float64)  
// AVX512: Process in multiples of 16 (float32) or 8 (float64)

// Use array interface for large datasets (>1000 elements)
// Use vector interface for small, known-size operations
```

### Architecture-Specific Optimizations
```cpp
// Compile with architecture-specific flags for maximum performance:
// -march=native -mtune=native
// 
// Or target specific architectures:
// -march=skylake-avx512  (Intel Skylake with AVX-512)
// -march=znver2          (AMD Zen 2)
```

---

This completes the comprehensive API reference for PRISM. For additional examples and usage patterns, see the main [README](../README.md) and [Getting Started Guide](GETTING_STARTED.md).