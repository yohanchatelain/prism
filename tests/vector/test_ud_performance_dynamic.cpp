#include <chrono>
#include <numeric>
#include <stddef.h>
#include <stdio.h>

#include "gtest/gtest.h"

#include "hwy/tests/hwy_gtest.h"
#include "hwy/tests/test_util-inl.h"

#include "src/ud_vector.h"

namespace prism::ud::vector {

constexpr size_t repetitions = 10000;

using VecArgf32 = hwy::AlignedUniquePtr<float[]>;
using VecArgf64 = hwy::AlignedUniquePtr<double[]>;

using binary_f32_op = void (*)(const VecArgf32 &, const VecArgf32 &,
                               const VecArgf32 &, const size_t);
using binary_f64_op = void (*)(const VecArgf64 &, const VecArgf64 &,
                               const VecArgf64 &, const size_t);

using ternary_f32_op = void (*)(const VecArgf32 &, const VecArgf32 &,
                                const VecArgf32 &, const VecArgf32 &,
                                const size_t);

using ternary_f64_op = void (*)(const VecArgf64 &, const VecArgf64 &,
                                const VecArgf64 &, const VecArgf64 &,
                                const size_t);

template <typename T> constexpr auto GetFormatString() -> const char * {
  if constexpr (std::is_same_v<T, float>) {
    return "%+.6a ";
  } else {
    return "%+.13a ";
  }
}

template <std::size_t S, typename T, typename Op, std::size_t arity = 2,
          std::size_t N = repetitions>
void MeasureFunction(Op func, const std::size_t lanes = 0,
                     const bool verbose = false) {

  constexpr size_t inputs_size = S;
  const auto a = hwy::MakeUniqueAlignedArray<T>(inputs_size);
  const auto b = hwy::MakeUniqueAlignedArray<T>(inputs_size);
  const auto c = hwy::MakeUniqueAlignedArray<T>(inputs_size);
  auto r = hwy::MakeUniqueAlignedArray<T>(inputs_size);
  constexpr T ulp = std::is_same_v<T, float> ? 0x1.0p-24F : 0x1.0p-53;
  constexpr const char *fmt = GetFormatString<T>();

  for (size_t i = 0; i < inputs_size; i++) {
    a[i] = 1.0;
    b[i] = ulp;
    c[i] = 0.1 * ulp;
  }

  std::vector<double> times(N);

  for (size_t i = 0; i < N; i++) {
    auto start = std::chrono::high_resolution_clock::now();
    if constexpr (arity == 1) {
      func(a, r, inputs_size);
    } else if constexpr (arity == 2) {
      func(a, b, r, inputs_size);
    } else if constexpr (arity == 3) {
      func(a, b, c, r, inputs_size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    times[i] = diff.count();
    if (verbose) {
      std::cout << "Iteration: " << i << " time: " << diff.count() << " s\n";
    }

    if (verbose) {
      for (size_t i = 0; i < inputs_size; i++) {
        fprintf(stderr, fmt, r[i]);
        if ((lanes != 0) and ((i % lanes) == (lanes - 1))) {
          fprintf(stderr, "\n");
        }
      }
      fprintf(stderr, "\n");
    }
  }

  // min, median, mean, max
  auto min = *std::min_element(times.begin(), times.end());
  auto max = *std::max_element(times.begin(), times.end());
  auto mean = std::accumulate(times.begin(), times.end(), 0.0) / N;
  auto std = std::sqrt(
      std::inner_product(times.begin(), times.end(), times.begin(), 0.0) / N -
      mean * mean);

  fprintf(stderr, "[%-4zu] ", inputs_size);
  fprintf(stderr, "%.4e ± %.4e [%.4e - %.4e] (%zu)\n", mean, std, min, max, N);
}

template <std::size_t S, typename T, typename V, typename Op,
          std::size_t arity = 2, std::size_t N = repetitions>
void MeasureFunctionX(Op func, const std::size_t lanes = 0,
                      const bool verbose = false) {

  constexpr size_t inputs_size = S;
  constexpr T ulp = std::is_same_v<T, float> ? 0x1.0p-24 : 0x1.0p-53;
  V a = {0};
  V b = {0};
  V c = {0};
  constexpr const char *fmt = GetFormatString<T>();

  for (size_t i = 0; i < inputs_size; i++) {
    a[i] = 1.0;
    b[i] = ulp;
    c[i] = 0.1 * ulp;
  }

  std::vector<double> times(N);

  for (size_t i = 0; i < N; i++) {
    auto start = std::chrono::high_resolution_clock::now();
    V r;
    if constexpr (arity == 1) {
      r = func(a, inputs_size);
    } else if constexpr (arity == 2) {
      r = func(a, b, inputs_size);
    } else if constexpr (arity == 3) {
      r = func(a, b, c, inputs_size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    times[i] = diff.count();
    if (verbose) {
      std::cout << "Iteration: " << i << " time: " << diff.count() << " s\n";
    }

    if (verbose) {
      for (size_t j = 0; j < inputs_size; j++) {
        fprintf(stderr, fmt, ((T *)&r)[j]);
        if ((lanes != 0) and ((j % lanes) == (lanes - 1))) {
          fprintf(stderr, "\n");
        }
      }
      fprintf(stderr, "\n");
    }
  }

  // min, median, mean, max
  auto min = *std::min_element(times.begin(), times.end());
  auto max = *std::max_element(times.begin(), times.end());
  auto mean = std::accumulate(times.begin(), times.end(), 0.0) / N;
  auto std = std::sqrt(
      std::inner_product(times.begin(), times.end(), times.begin(), 0.0) / N -
      mean * mean);

  fprintf(stderr, "[%-4zu] ", inputs_size);
  fprintf(stderr, "%.4e ± %.4e [%.4e - %.4e] (%zu)\n", mean, std, min, max, N);
}

/* Test array inputs */

#define define_array_test_un(op, type)                                         \
  void test_##op##type(const VecArg##type &a, VecArg##type &r,                 \
                       const size_t count) {                                   \
    op##type(a.get(), r.get(), count);                                         \
  }

#define define_array_test_bin(op, type)                                        \
  void test_##op##type(const VecArg##type &a, const VecArg##type &b,           \
                       const VecArg##type &c, const size_t count) {            \
    op##type(a.get(), b.get(), c.get(), count);                                \
  }

#define define_array_test_ter(op, type)                                        \
  void test_##op##type(const VecArg##type &a, const VecArg##type &b,           \
                       const VecArg##type &c, const VecArg##type &d,           \
                       const size_t count) {                                   \
    op##type(a.get(), b.get(), c.get(), d.get(), count);                       \
  }

namespace dynamic_dispatch {

define_array_test_un(sqrt, f32);
define_array_test_bin(add, f32);
define_array_test_bin(sub, f32);
define_array_test_bin(mul, f32);
define_array_test_bin(div, f32);
define_array_test_ter(fma, f32);

define_array_test_un(sqrt, f64);
define_array_test_bin(add, f64);
define_array_test_bin(sub, f64);
define_array_test_bin(mul, f64);
define_array_test_bin(div, f64);
define_array_test_ter(fma, f64);

/* Test vector inputs (dynamic dispatch) */

#define define_vector_test_dynamic_un(op, type, count)                         \
  void test_##op##type##x##count##_d(const VecArg##type &a, VecArg##type &r,   \
                                     int) {                                    \
    op##type##x##count(a.get(), r.get());                                      \
  }

#define define_vector_test_dynamic_bin(op, type, count)                        \
  void test_##op##type##x##count##_d(const VecArg##type &a,                    \
                                     const VecArg##type &b,                    \
                                     const VecArg##type &r, int) {             \
    op##type##x##count(a.get(), b.get(), r.get());                             \
  }

#define define_vector_test_dynamic_ter(op, type, count)                        \
  void test_##op##type##x##count##_d(                                          \
      const VecArg##type &a, const VecArg##type &b, const VecArg##type &c,     \
      const VecArg##type &r, int) {                                            \
    op##type##x##count(a.get(), b.get(), c.get(), r.get());                    \
  }

define_vector_test_dynamic_un(sqrt, f32, 2);
define_vector_test_dynamic_bin(add, f32, 2);
define_vector_test_dynamic_bin(sub, f32, 2);
define_vector_test_dynamic_bin(mul, f32, 2);
define_vector_test_dynamic_bin(div, f32, 2);
define_vector_test_dynamic_ter(fma, f32, 2);

define_vector_test_dynamic_un(sqrt, f64, 2);
define_vector_test_dynamic_bin(add, f64, 2);
define_vector_test_dynamic_bin(sub, f64, 2);
define_vector_test_dynamic_bin(mul, f64, 2);
define_vector_test_dynamic_bin(div, f64, 2);
define_vector_test_dynamic_ter(fma, f64, 2);

define_vector_test_dynamic_un(sqrt, f32, 4);
define_vector_test_dynamic_bin(add, f32, 4);
define_vector_test_dynamic_bin(sub, f32, 4);
define_vector_test_dynamic_bin(mul, f32, 4);
define_vector_test_dynamic_bin(div, f32, 4);
define_vector_test_dynamic_ter(fma, f32, 4);

define_vector_test_dynamic_un(sqrt, f64, 4);
define_vector_test_dynamic_bin(add, f64, 4);
define_vector_test_dynamic_bin(sub, f64, 4);
define_vector_test_dynamic_bin(mul, f64, 4);
define_vector_test_dynamic_bin(div, f64, 4);
define_vector_test_dynamic_ter(fma, f64, 4);

define_vector_test_dynamic_un(sqrt, f32, 8);
define_vector_test_dynamic_bin(add, f32, 8);
define_vector_test_dynamic_bin(sub, f32, 8);
define_vector_test_dynamic_bin(mul, f32, 8);
define_vector_test_dynamic_bin(div, f32, 8);
define_vector_test_dynamic_ter(fma, f32, 8);

define_vector_test_dynamic_un(sqrt, f64, 8);
define_vector_test_dynamic_bin(add, f64, 8);
define_vector_test_dynamic_bin(sub, f64, 8);
define_vector_test_dynamic_bin(mul, f64, 8);
define_vector_test_dynamic_bin(div, f64, 8);
define_vector_test_dynamic_ter(fma, f64, 8);

define_vector_test_dynamic_un(sqrt, f32, 16);
define_vector_test_dynamic_bin(add, f32, 16);
define_vector_test_dynamic_bin(sub, f32, 16);
define_vector_test_dynamic_bin(mul, f32, 16);
define_vector_test_dynamic_bin(div, f32, 16);
define_vector_test_dynamic_ter(fma, f32, 16);
}; // namespace dynamic_dispatch

// Recursion function to call MeasureFunction with powers of 2
template <size_t N, size_t Max, typename T, size_t A, typename Op>
void callMeasureFunctions(Op function) {
  MeasureFunction<N, T, Op, A>(function);
  if constexpr (N * 2 <= Max) {
    callMeasureFunctions<N * 2, Max, T, A, Op>(function);
  }
}

namespace dynamic_dispatch {

/* Test on Array inputs */

/* IEEE-754 binary32 */

TEST(UDArrayBenchmark, UDAddF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2, decltype(&test_addf32)>(&test_addf32);
}

TEST(UDArrayBenchmark, UDSubF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_subf32);
}

TEST(UDArrayBenchmark, UDMulF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_mulf32);
}

TEST(UDArrayBenchmark, UDDivF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_divf32);
}

TEST(UDArrayBenchmark, UDSqrtF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 1>(&test_sqrtf32);
}

TEST(UDArrayBenchmark, UDFmaF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 3>(&test_fmaf32);
}

/* IEEE-754 binary64 */

TEST(UDArrayBenchmark, UDAddF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_addf64);
}

TEST(UDArrayBenchmark, UDSubF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_subf64);
}

TEST(UDArrayBenchmark, UDMulF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_mulf64);
}

TEST(UDArrayBenchmark, UDDivF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_divf64);
}

TEST(UDArrayBenchmark, UDSqrtF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 1>(&test_sqrtf64);
}

TEST(UDArrayBenchmark, UDFmaF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 3>(&test_fmaf64);
}

const auto kVerbose = false;

/* Test on single vector passed by value with dynamic dispatch */

TEST(UDVectorDynamicBenchmark, UDAddF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_addf32x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_subf32x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_mulf32x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_divf32x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x2_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x2_d);
  MeasureFunction<2, float, Op, 1>(&test_sqrtf32x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x2_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x2_d);
  MeasureFunction<2, float, Op, 3>(&test_fmaf32x2_d, 2, kVerbose);
}

/* IEEE-754 binary64 x2 */

TEST(UDVectorDynamicBenchmark, UDAddF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_addf64x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_subf64x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_mulf64x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_divf64x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x2_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x2_d);
  MeasureFunction<2, double, Op, 1>(&test_sqrtf64x2_d, 2, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x2_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x2_d);
  MeasureFunction<2, double, Op, 3>(&test_fmaf64x2_d, 2, kVerbose);
}

/* IEEE-754 binary32 x4 */

TEST(UDVectorDynamicBenchmark, UDAddF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_addf32x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_subf32x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_mulf32x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_divf32x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x4_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x4_d);
  MeasureFunction<4, float, Op, 1>(&test_sqrtf32x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x4_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x4_d);
  MeasureFunction<4, float, Op, 3>(&test_fmaf32x4_d, 4, kVerbose);
}

/* IEEE-754 binary32 x8 */

TEST(UDVectorDynamicBenchmark, UDAddF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_addf32x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_subf32x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_mulf32x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_divf32x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x8_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x8_d);
  MeasureFunction<8, float, Op, 1>(&test_sqrtf32x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x8_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x8_d);
  MeasureFunction<8, float, Op, 3>(&test_fmaf32x8_d, 8, kVerbose);
}

/* IEEE-754 binary64 x4 */

TEST(UDVectorDynamicBenchmark, UDAddF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_addf64x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_subf64x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_mulf64x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_divf64x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x4_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x4_d);
  MeasureFunction<4, double, Op, 1>(&test_sqrtf64x4_d, 4, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x4_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x4_d);
  MeasureFunction<4, double, Op, 3>(&test_fmaf64x4_d, 4, kVerbose);
}

/* IEEE-754 binary64 x8 */

TEST(UDVectorDynamicBenchmark, UDAddF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_addf64x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_subf64x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_mulf64x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_divf64x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x8_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x8_d);
  MeasureFunction<8, double, Op, 1>(&test_sqrtf64x8_d, 8, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x8_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x8_d);
  MeasureFunction<8, double, Op, 3>(&test_fmaf64x8_d, 8, kVerbose);
}

/* IEEE-754 binary32 x16 */

TEST(UDVectorDynamicBenchmark, UDAddF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_addf32x16_d, 16, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSubF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_subf32x16_d, 16, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDMulF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_mulf32x16_d, 16, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDDivF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_divf32x16_d, 16, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDSqrtF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x16_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x16_d);
  MeasureFunction<16, float, Op, 1>(&test_sqrtf32x16_d, 16, kVerbose);
}

TEST(UDVectorDynamicBenchmark, UDFmaF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x16_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_fmaf32x16_d);
  MeasureFunction<16, float, Op, 3>(&test_fmaf32x16_d, 16, kVerbose);
}
}; // namespace dynamic_dispatch

} // namespace prism::ud::vector

// HYW_TEST_MAIN();1
