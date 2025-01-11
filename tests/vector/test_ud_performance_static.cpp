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

namespace static_dispatch {

/* Test vector inputs (static dispatch) */

#define define_vector_test_un(op, type, size)                                  \
  type##x##size##_v test_##op##type##x##size##_v(const type##x##size##_v a,    \
                                                 const size_t count) {         \
    return op##type##x##size(a);                                               \
  }

#define define_vector_test_bin(op, type, size)                                 \
  type##x##size##_v test_##op##type##x##size##_v(const type##x##size##_v a,    \
                                                 const type##x##size##_v b,    \
                                                 const size_t count) {         \
    return op##type##x##size(a, b);                                            \
  }

#define define_vector_test_ter(op, type, size)                                 \
  type##x##size##_v test_##op##type##x##size##_v(                              \
      const type##x##size##_v a, const type##x##size##_v b,                    \
      const type##x##size##_v c, const size_t count) {                         \
    return op##type##x##size(a, b, c);                                         \
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

}; // namespace static_dispatch

// Recursion function to call MeasureFunction with powers of 2
template <size_t N, size_t Max, typename T, size_t A, typename Op>
void callMeasureFunctions(Op function) {
  MeasureFunction<N, T, Op, A>(function);
  if constexpr (N * 2 <= Max) {
    callMeasureFunctions<N * 2, Max, T, A, Op>(function);
  }
}

namespace static_dispatch {
/* Test on single vector passed by value with static dispatch */

/* IEEE-754 binary32 x2 */
const auto kVerbose = false;

TEST(UDVectorBenchmark, UDAddF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_addf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDAddF32x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_subf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDSubF32x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_mulf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDMulF32x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, float, f32x2_v>(&test_divf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDDivF32x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x2_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x2_v);
  MeasureFunctionX<2, float, f32x2_v, Op, 1>(&test_sqrtf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF32x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF32x2Static) {
#if HWY_MAX_BYTES >= 8
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x2_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x2_v);
  MeasureFunctionX<2, float, f32x2_v, Op, 3>(&test_fmaf32x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF32x2Static is not available";
#endif
}

/* IEEE-754 binary64 x2 */
TEST(UDVectorBenchmark, UDAddF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_addf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDAddF64x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_subf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDSubF64x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_mulf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDMulF64x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x2_v with " << N << " repetitions\n";
  MeasureFunctionX<2, double, f64x2_v>(&test_divf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDDivF64x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x2_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x2_v);
  MeasureFunctionX<2, double, f64x2_v, Op, 1>(&test_sqrtf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF64x2Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF64x2Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x2_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x2_v);
  MeasureFunctionX<2, double, f64x2_v, Op, 3>(&test_fmaf64x2_v, 2, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF64x2Static is not available";
#endif
}

/* IEEE-754 binary32 x4 */

TEST(UDVectorBenchmark, UDAddF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_addf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDAddF32x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_subf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDSubF32x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_mulf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDMulF32x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, float, f32x4_v>(&test_divf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDDivF32x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x4_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x4_v);
  MeasureFunctionX<4, float, f32x4_v, Op, 1>(&test_sqrtf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF32x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF32x4Static) {
#if HWY_MAX_BYTES >= 16
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x4_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x4_v);
  MeasureFunctionX<4, float, f32x4_v, Op, 3>(&test_fmaf32x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF32x4Static is not available";
#endif
}

/* IEEE-754 binary32 x8 */

TEST(UDVectorBenchmark, UDAddF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_addf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDAddF32x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_subf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDSubF32x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_mulf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDMulF32x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, float, f32x8_v>(&test_divf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDDivF32x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x8_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x8_v);
  MeasureFunctionX<8, float, f32x8_v, Op, 1>(&test_sqrtf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF32x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF32x8Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x8_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf32x8_v);
  MeasureFunctionX<8, float, f32x8_v, Op, 3>(&test_fmaf32x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF32x8Static is not available";
#endif
}

/* IEEE-754 binary64 x4 */

TEST(UDVectorBenchmark, UDAddF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_addf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDAddF64x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_subf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDSubF64x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_mulf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDMulF64x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x4_v with " << N << " repetitions\n";
  MeasureFunctionX<4, double, f64x4_v>(&test_divf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDDivF64x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x4_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x4_v);
  MeasureFunctionX<4, double, f64x4_v, Op, 1>(&test_sqrtf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF64x4Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF64x4Static) {
#if HWY_MAX_BYTES >= 32
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x4_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x4_v);
  MeasureFunctionX<4, double, f64x4_v, Op, 3>(&test_fmaf64x4_v, 4, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF64x4Static is not available";
#endif
}

/* IEEE-754 binary64 x8 */

TEST(UDVectorBenchmark, UDAddF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_addf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDAddF64x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_subf64x8_v, 8, kVerbose);

#else
  GTEST_SKIP() << "UDSubF64x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_mulf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDMulF64x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf64x8_v with " << N << " repetitions\n";
  MeasureFunctionX<8, double, f64x8_v>(&test_divf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDDivF64x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf64x8_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf64x8_v);
  MeasureFunctionX<8, double, f64x8_v, Op, 1>(&test_sqrtf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF64x8Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF64x8Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf64x8_v with " << N << " repetitions\n";
  using Op = decltype(&test_fmaf64x8_v);
  MeasureFunctionX<8, double, f64x8_v, Op, 3>(&test_fmaf64x8_v, 8, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF64x8Static is not available";
#endif
}

/* IEEE-754 binary32 x16 */

TEST(UDVectorBenchmark, UDAddF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::addf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_addf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "UDAddF32x16Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSubF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::subf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_subf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "UDSubF32x16Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDMulF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::mulf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_mulf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "UDMulF32x16Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDDivF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::divf32x16_v with " << N
            << " repetitions\n";
  MeasureFunctionX<16, float, f32x16_v>(&test_divf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "UDDivF32x16Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDSqrtF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::sqrtf32x16_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_sqrtf32x16_v);
  MeasureFunctionX<16, float, f32x16_v, Op, 1>(&test_sqrtf32x16_v, 16,
                                               kVerbose);
#else
  GTEST_SKIP() << "UDSqrtF32x16Static is not available";
#endif
}

TEST(UDVectorBenchmark, UDFmaF32x16Static) {
#if HWY_MAX_BYTES >= 64
  constexpr size_t N = repetitions;
  std::cout << "Measure function ud::fmaf32x16_v with " << N
            << " repetitions\n";
  using Op = decltype(&test_fmaf32x16_v);
  MeasureFunctionX<16, float, f32x16_v, Op, 3>(&test_fmaf32x16_v, 16, kVerbose);
#else
  GTEST_SKIP() << "UDFmaF32x16Static is not available";
#endif
}
}; // namespace static_dispatch

} // namespace prism::ud::vector

// HYW_TEST_MAIN();1
