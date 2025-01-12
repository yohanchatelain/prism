#ifdef PRISM_IDE
#include "hwy/highway.h"
#undef HWY_MAX_BYTES
#define HWY_MAX_BYTES UINT32_MAX
#endif

namespace variable {

/* Variable size functions */

/* IEEE-754 binary32 */

void addf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, size_t count);

void subf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, size_t count);

void mulf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, size_t count);

void divf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, size_t count);

void sqrtf32(const float *HWY_RESTRICT a, float *HWY_RESTRICT result,
             size_t count);

void fmaf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            const float *HWY_RESTRICT c, float *HWY_RESTRICT result,
            size_t count);

/* IEEE-754 binary64 */

void addf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, size_t count);

void subf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, size_t count);

void mulf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, size_t count);

void divf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, size_t count);

void sqrtf64(const double *HWY_RESTRICT a, double *HWY_RESTRICT result,
             size_t count);

void fmaf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            const double *HWY_RESTRICT c, double *HWY_RESTRICT result,
            size_t count);

} // namespace variable

namespace fixed {

/* Fixed size functions */

typedef float f32x2_v __attribute__((vector_size(8)));
typedef double f64x2_v __attribute__((vector_size(16)));
typedef float f32x4_v __attribute__((vector_size(16)));
typedef double f64x4_v __attribute__((vector_size(32)));
typedef float f32x8_v __attribute__((vector_size(32)));
typedef double f64x8_v __attribute__((vector_size(64)));
typedef float f32x16_v __attribute__((vector_size(64)));
typedef double f64x16_v __attribute__((vector_size(128)));

/* 64-bits */

#if HWY_MAX_BYTES >= 8
auto addf32x2(f32x2_v a, f32x2_v b) -> f32x2_v;
auto subf32x2(f32x2_v a, f32x2_v b) -> f32x2_v;
auto mulf32x2(f32x2_v a, f32x2_v b) -> f32x2_v;
auto divf32x2(f32x2_v a, f32x2_v b) -> f32x2_v;
auto sqrtf32x2(f32x2_v a) -> f32x2_v;
auto fmaf32x2(f32x2_v a, f32x2_v b, f32x2_v c) -> f32x2_v;
#endif

/* 128-bits */
#if HWY_MAX_BYTES >= 16
auto addf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto subf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto mulf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto divf64x2(f64x2_v a, f64x2_v b) -> f64x2_v;
auto sqrtf64x2(f64x2_v a) -> f64x2_v;
auto fmaf64x2(f64x2_v a, f64x2_v b, f64x2_v c) -> f64x2_v;

auto addf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto subf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto mulf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto divf32x4(f32x4_v a, f32x4_v b) -> f32x4_v;
auto sqrtf32x4(f32x4_v a) -> f32x4_v;
auto fmaf32x4(f32x4_v a, f32x4_v b, f32x4_v c) -> f32x4_v;
#endif

/* 256-bits */
#if HWY_MAX_BYTES >= 32
auto addf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto subf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto mulf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto divf64x4(f64x4_v a, f64x4_v b) -> f64x4_v;
auto sqrtf64x4(f64x4_v a) -> f64x4_v;
auto fmaf64x4(f64x4_v a, f64x4_v b, f64x4_v c) -> f64x4_v;

auto addf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto subf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto mulf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto divf32x8(f32x8_v a, f32x8_v b) -> f32x8_v;
auto sqrtf32x8(f32x8_v a) -> f32x8_v;
auto fmaf32x8(f32x8_v a, f32x8_v b, f32x8_v c) -> f32x8_v;
#endif

/* 512-bits */
#if HWY_MAX_BYTES >= 64
auto addf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto subf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto mulf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto divf64x8(f64x8_v a, f64x8_v b) -> f64x8_v;
auto sqrtf64x8(f64x8_v a) -> f64x8_v;
auto fmaf64x8(f64x8_v a, f64x8_v b, f64x8_v c) -> f64x8_v;

auto addf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto subf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto mulf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto divf32x16(f32x16_v a, f32x16_v b) -> f32x16_v;
auto sqrtf32x16(f32x16_v a) -> f32x16_v;
auto fmaf32x16(f32x16_v a, f32x16_v b, f32x16_v c) -> f32x16_v;
#endif

/* 1024-bits */
#if HWY_MAX_BYTES >= 128
auto addf64x16(f64x16_v a, f64x16_v b) -> f64x16_v;
auto subf64x16(f64x16_v a, f64x16_v b) -> f64x16_v;
auto mulf64x16(f64x16_v a, f64x16_v b) -> f64x16_v;
auto divf64x16(f64x16_v a, f64x16_v b) -> f64x16_v;
auto sqrtf64x16(f64x16_v a) -> f64x16_v;
auto fmaf64x16(f64x16_v a, f64x16_v b, f64x16_v c) -> f64x16_v;
#endif

} // namespace fixed