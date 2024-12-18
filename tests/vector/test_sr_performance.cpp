#include "hwy/nanobenchmark.h"

#include <chrono>
#include <numeric>
#include <stddef.h>
#include <stdio.h>

#include "gtest/gtest.h"

#include "hwy/tests/hwy_gtest.h"
#include "hwy/tests/test_util-inl.h"

#include "src/sr_vector.h"
namespace prism::sr::vector {

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

template <typename T> constexpr const char *GetFormatString() {
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
  constexpr T ulp = std::is_same_v<T, float> ? 0x1.0p-24f : 0x1.0p-53;
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
    if (verbose)
      std::cout << "Iteration: " << i << " time: " << diff.count() << " s\n";

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
    if (verbose)
      std::cout << "Iteration: " << i << " time: " << diff.count() << " s\n";

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

/* Test vector inputs (static dispatch) */

#define define_vector_test_un(op, type, size)                                  \
  type##x##size##_v test_##op##type##x##size##_v(const type##x##size##_v a,    \
                                                 const size_t count) {         \
    return op##type##x##size##_static(a);                                      \
  }

#define define_vector_test_bin(op, type, size)                                 \
  type##x##size##_v test_##op##type##x##size##_v(const type##x##size##_v a,    \
                                                 const type##x##size##_v b,    \
                                                 const size_t count) {         \
    return op##type##x##size##_static(a, b);                                   \
  }

#define define_vector_test_ter(op, type, size)                                 \
  type##x##size##_v test_##op##type##x##size##_v(                              \
      const type##x##size##_v a, const type##x##size##_v b,                    \
      const type##x##size##_v c, const size_t count) {                         \
    return op##type##x##size##_static(a, b, c);                                \
  }

/* 64-bits */

#if HWY_MAX_BYTES >= 8
define_vector_test_un(sqrt, f32, 2);
define_vector_test_bin(add, f32, 2);
define_vector_test_bin(sub, f32, 2);
define_vector_test_bin(mul, f32, 2);
define_vector_test_bin(div, f32, 2);
define_vector_test_ter(fma, f32, 2);
#endif

/* 128-bits */

#if HWY_MAX_BYTES >= 16
define_vector_test_un(sqrt, f64, 2);
define_vector_test_bin(add, f64, 2);
define_vector_test_bin(sub, f64, 2);
define_vector_test_bin(mul, f64, 2);
define_vector_test_bin(div, f64, 2);
define_vector_test_ter(fma, f64, 2);

define_vector_test_un(sqrt, f32, 4);
define_vector_test_bin(add, f32, 4);
define_vector_test_bin(sub, f32, 4);
define_vector_test_bin(mul, f32, 4);
define_vector_test_bin(div, f32, 4);
define_vector_test_ter(fma, f32, 4);
#endif

/* 256-bits */
#if HWY_MAX_BYTES >= 32
define_vector_test_un(sqrt, f64, 4);
define_vector_test_bin(add, f64, 4);
define_vector_test_bin(sub, f64, 4);
define_vector_test_bin(mul, f64, 4);
define_vector_test_bin(div, f64, 4);
define_vector_test_ter(fma, f64, 4);

define_vector_test_un(sqrt, f32, 8);
define_vector_test_bin(add, f32, 8);
define_vector_test_bin(sub, f32, 8);
define_vector_test_bin(mul, f32, 8);
define_vector_test_bin(div, f32, 8);
define_vector_test_ter(fma, f32, 8);
#endif

/* 512-bits */
#if HWY_MAX_BYTES >= 64
define_vector_test_un(sqrt, f64, 8);
define_vector_test_bin(add, f64, 8);
define_vector_test_bin(sub, f64, 8);
define_vector_test_bin(mul, f64, 8);
define_vector_test_bin(div, f64, 8);
define_vector_test_ter(fma, f64, 8);

define_vector_test_un(sqrt, f32, 16);
define_vector_test_bin(add, f32, 16);
define_vector_test_bin(sub, f32, 16);
define_vector_test_bin(mul, f32, 16);
define_vector_test_bin(div, f32, 16);
define_vector_test_ter(fma, f32, 16);
#endif

/* Test vector inputs (dynamic dispatch) */

#define define_vector_test_dynamic_un(op, type, count)                         \
  void test_##op##type##x##count##_d(const VecArg##type &a, VecArg##type &r,   \
                                     int) {                                    \
    op##type##x##count##_dynamic(a.get(), r.get());                            \
  }

#define define_vector_test_dynamic_bin(op, type, count)                        \
  void test_##op##type##x##count##_d(const VecArg##type &a,                    \
                                     const VecArg##type &b,                    \
                                     const VecArg##type &r, int) {             \
    op##type##x##count##_dynamic(a.get(), b.get(), r.get());                   \
  }

#define define_vector_test_dynamic_ter(op, type, count)                        \
  void test_##op##type##x##count##_d(                                          \
      const VecArg##type &a, const VecArg##type &b, const VecArg##type &c,     \
      const VecArg##type &r, int) {                                            \
    op##type##x##count##_dynamic(a.get(), b.get(), c.get(), r.get());          \
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

// Recursion function to call MeasureFunction with powers of 2
template <size_t N, size_t Max, typename T, size_t A, typename Op>
void callMeasureFunctions(Op function) {
  MeasureFunction<N, T, Op, A>(function);
  if constexpr (N * 2 <= Max) {
    callMeasureFunctions<N * 2, Max, T, A, Op>(function);
  }
}

/* Test on Array inputs */

/* IEEE-754 binary32 */

TEST(SRArrayBenchmark, SRAddF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2, decltype(&test_addf32)>(&test_addf32);
}

TEST(SRArrayBenchmark, SRSubF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_subf32);
}

TEST(SRArrayBenchmark, SRMulF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_mulf32);
}

TEST(SRArrayBenchmark, SRDivF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 2>(&test_divf32);
}

TEST(SRArrayBenchmark, SRSqrtF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 1>(&test_sqrtf32);
}

TEST(SRArrayBenchmark, SRFmaF32) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, float, 3>(&test_fmaf32);
}

/* IEEE-754 binary64 */

TEST(SRArrayBenchmark, SRAddF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_addf64);
}

TEST(SRArrayBenchmark, SRSubF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_subf64);
}

TEST(SRArrayBenchmark, SRMulF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_mulf64);
}

TEST(SRArrayBenchmark, SRDivF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 2>(&test_divf64);
}

TEST(SRArrayBenchmark, SRSqrtF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 1>(&test_sqrtf64);
}

TEST(SRArrayBenchmark, SRFmaF64) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64 with " << N << " repetitions\n";
  callMeasureFunctions<2, 1024, double, 3>(&test_fmaf64);
}

/* Test on single vector passed by value with static dispatch */

/* IEEE-754 binary32 x2 */

const auto kVerbose = false;

TEST(SRVectorBenchmark, SRAddF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_addf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRAddF32x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_subf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRSubF32x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_mulf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRMulF32x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_divf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRDivF32x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x2_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x2_v);
  MeasureFunctionX<2, float, f32x2_v, Op, 1>(&test_sqrtf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF32x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x2_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x2_v);
  MeasureFunctionX<2, float, f32x2_v, Op, 3>(&test_fmaf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF32x2Static is not available";
#endif
}

/* IEEE-754 binary64 x2 */
TEST(SRVectorBenchmark, SRAddF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_addf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRAddF64x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_subf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRSubF64x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_mulf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRMulF64x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_divf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRDivF64x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x2_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x2_v);
  MeasureFunctionX<2, double, f64x2_v, Op, 1>(&test_sqrtf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF64x2Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x2_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x2_v);
  MeasureFunctionX<2, double, f64x2_v, Op, 3>(&test_fmaf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF64x2Static is not available";
#endif
}

/* IEEE-754 binary32 x4 */

TEST(SRVectorBenchmark, SRAddF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_addf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRAddF32x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_subf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRSubF32x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_mulf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRMulF32x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_divf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRDivF32x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x4_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x4_v);
  MeasureFunctionX<4, float, f32x4_v, Op, 1>(&test_sqrtf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF32x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x4_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x4_v);
  MeasureFunctionX<4, float, f32x4_v, Op, 3>(&test_fmaf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF32x4Static is not available";
#endif
}

/* IEEE-754 binary32 x8 */

TEST(SRVectorBenchmark, SRAddF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_addf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRAddF32x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_subf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRSubF32x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_mulf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRMulF32x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_divf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRDivF32x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x8_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x8_v);
  MeasureFunctionX<8, float, f32x8_v, Op, 1>(&test_sqrtf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF32x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x8_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x8_v);
  MeasureFunctionX<8, float, f32x8_v, Op, 3>(&test_fmaf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF32x8Static is not available";
#endif
}

/* IEEE-754 binary64 x4 */

TEST(SRVectorBenchmark, SRAddF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_addf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRAddF64x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_subf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRSubF64x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_mulf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRMulF64x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_divf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRDivF64x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x4_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x4_v);
  MeasureFunctionX<4, double, f64x4_v, Op, 1>(&test_sqrtf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF64x4Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x4_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x4_v);
  MeasureFunctionX<4, double, f64x4_v, Op, 3>(&test_fmaf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF64x4Static is not available";
#endif
}

/* IEEE-754 binary64 x8 */

TEST(SRVectorBenchmark, SRAddF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_addf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRAddF64x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_subf64x8_v, 8, kVerbose);

#else
  GTEST_SKIP() << "SRSubF64x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_mulf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRMulF64x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_divf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRDivF64x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x8_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x8_v);
  MeasureFunctionX<8, double, f64x8_v, Op, 1>(&test_sqrtf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF64x8Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x8_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x8_v);
  MeasureFunctionX<8, double, f64x8_v, Op, 3>(&test_fmaf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF64x8Static is not available";
#endif
}

/* IEEE-754 binary32 x16 */

TEST(SRVectorBenchmark, SRAddF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_addf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "SRAddF32x16Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSubF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_subf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "SRSubF32x16Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRMulF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_mulf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "SRMulF32x16Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRDivF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_divf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "SRDivF32x16Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRSqrtF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x16_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x16_v);
  MeasureFunctionX<16, float, f32x16_v, Op, 1>(&test_sqrtf32x16_v, 16,
                                               kVerbose);
#else
  GTEST_SKIP() << "SRSqrtF32x16Static is not available";
#endif
}

TEST(SRVectorBenchmark, SRFmaF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x16_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_fmaf32x16_v);
  MeasureFunctionX<16, float, f32x16_v, Op, 3>(&test_fmaf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "SRFmaF32x16Static is not available";
#endif
}

/* Test on single vector passed by value with dynamic dispatch */

TEST(SRVectorDynamicBenchmark, SRAddF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_addf32x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_subf32x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_mulf32x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x2_d with " << N << " repetitions\n";
  MeasureFunction<2, float>(&test_divf32x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x2_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x2_d);
  MeasureFunction<2, float, Op, 1>(&test_sqrtf32x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF32x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x2_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x2_d);
  MeasureFunction<2, float, Op, 3>(&test_fmaf32x2_d, 2, kVerbose);
}

/* IEEE-754 binary64 x2 */

TEST(SRVectorDynamicBenchmark, SRAddF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_addf64x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_subf64x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_mulf64x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x2_d with " << N << " repetitions\n";
  MeasureFunction<2, double>(&test_divf64x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x2_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x2_d);
  MeasureFunction<2, double, Op, 1>(&test_sqrtf64x2_d, 2, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF64x2D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x2_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x2_d);
  MeasureFunction<2, double, Op, 3>(&test_fmaf64x2_d, 2, kVerbose);
}

/* IEEE-754 binary32 x4 */

TEST(SRVectorDynamicBenchmark, SRAddF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_addf32x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_subf32x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_mulf32x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x4_d with " << N << " repetitions\n";
  MeasureFunction<4, float>(&test_divf32x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x4_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x4_d);
  MeasureFunction<4, float, Op, 1>(&test_sqrtf32x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF32x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x4_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x4_d);
  MeasureFunction<4, float, Op, 3>(&test_fmaf32x4_d, 4, kVerbose);
}

/* IEEE-754 binary32 x8 */

TEST(SRVectorDynamicBenchmark, SRAddF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_addf32x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_subf32x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_mulf32x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x8_d with " << N << " repetitions\n";
  MeasureFunction<8, float>(&test_divf32x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x8_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x8_d);
  MeasureFunction<8, float, Op, 1>(&test_sqrtf32x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF32x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x8_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x8_d);
  MeasureFunction<8, float, Op, 3>(&test_fmaf32x8_d, 8, kVerbose);
}

/* IEEE-754 binary64 x4 */

TEST(SRVectorDynamicBenchmark, SRAddF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_addf64x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_subf64x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_mulf64x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x4_d with " << N << " repetitions\n";
  MeasureFunction<4, double>(&test_divf64x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x4_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x4_d);
  MeasureFunction<4, double, Op, 1>(&test_sqrtf64x4_d, 4, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF64x4D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x4_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x4_d);
  MeasureFunction<4, double, Op, 3>(&test_fmaf64x4_d, 4, kVerbose);
}

/* IEEE-754 binary64 x8 */

TEST(SRVectorDynamicBenchmark, SRAddF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_addf64x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_subf64x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_mulf64x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf64x8_d with " << N << " repetitions\n";
  MeasureFunction<8, double>(&test_divf64x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf64x8_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x8_d);
  MeasureFunction<8, double, Op, 1>(&test_sqrtf64x8_d, 8, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF64x8D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf64x8_d with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x8_d);
  MeasureFunction<8, double, Op, 3>(&test_fmaf64x8_d, 8, kVerbose);
}

/* IEEE-754 binary32 x16 */

TEST(SRVectorDynamicBenchmark, SRAddF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::addf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_addf32x16_d, 16, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSubF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::subf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_subf32x16_d, 16, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRMulF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::mulf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_mulf32x16_d, 16, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRDivF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::divf32x16_d with " << N
            << " repetitions\n";
  MeasureFunction<16, float>(&test_divf32x16_d, 16, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRSqrtF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::sqrtf32x16_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x16_d);
  MeasureFunction<16, float, Op, 1>(&test_sqrtf32x16_d, 16, kVerbose);
}

TEST(SRVectorDynamicBenchmark, SRFmaF32x16D) {
  constexpr size_t N = repetitions;
  std::cout << "Measure function sr::fmaf32x16_d with " << N
            << " repetitions\n";
  using Op = decltype(&test_fmaf32x16_d);
  MeasureFunction<16, float, Op, 3>(&test_fmaf32x16_d, 16, kVerbose);
}

} // namespace prism::sr::vector

// HYW_TEST_MAIN();1
