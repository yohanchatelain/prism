#ifndef __PRISM_SR_HW_H__
#define __PRISM_SR_HW_H__

#include "hwy/highway.h"

namespace prism::sr::vector {

namespace dynamic_dispatch {

/* Array functions */

/* IEEE-754 binary32 */

void addf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count);

void subf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count);

void mulf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count);

void divf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count);

void sqrtf32(const float *HWY_RESTRICT a, float *HWY_RESTRICT result,
             const size_t count);

void fmaf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            const float *HWY_RESTRICT c, float *HWY_RESTRICT result,
            const size_t count);

/* IEEE-754 binary64 */

void addf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count);

void subf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count);

void mulf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count);

void divf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count);

void sqrtf64(const double *HWY_RESTRICT a, double *HWY_RESTRICT result,
             const size_t count);

void fmaf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            const double *HWY_RESTRICT c, double *HWY_RESTRICT result,
            const size_t count);

/* Single vector functions with dynamic dispatch */

/* 64-bits */

/* IEEE-754 binary32 x2 */
void addf32x2(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void subf32x2(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void mulf32x2(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void divf32x2(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void sqrtf32x2(const float *HWY_RESTRICT a, float *HWY_RESTRICT result);
void fmaf32x2(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              const float *HWY_RESTRICT c, float *HWY_RESTRICT result);

/* 128-bits */

/* IEEE-754 binary32 x4 */
void addf32x4(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void subf32x4(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void mulf32x4(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void divf32x4(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void sqrtf32x4(const float *HWY_RESTRICT a, float *HWY_RESTRICT result);
void fmaf32x4(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              const float *HWY_RESTRICT c, float *HWY_RESTRICT result);

/* IEEE-754 binary64 x2 */
void addf64x2(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void subf64x2(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void mulf64x2(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void divf64x2(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void sqrtf64x2(const double *HWY_RESTRICT a, double *HWY_RESTRICT result);
void fmaf64x2(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              const double *HWY_RESTRICT c, double *HWY_RESTRICT result);

/* 256-bits */

/* IEEE-754 binary64 x4 */
void addf64x4(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void subf64x4(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void mulf64x4(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void divf64x4(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void sqrtf64x4(const double *HWY_RESTRICT a, double *HWY_RESTRICT result);
void fmaf64x4(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              const double *HWY_RESTRICT c, double *HWY_RESTRICT result);

/* IEEE-754 binary32 x8 */
void addf32x8(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void subf32x8(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void mulf32x8(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void divf32x8(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result);
void sqrtf32x8(const float *HWY_RESTRICT a, float *HWY_RESTRICT result);
void fmaf32x8(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              const float *HWY_RESTRICT c, float *HWY_RESTRICT result);

/* 512-bits */

/* IEEE-754 binary64 x8 */
void addf64x8(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void subf64x8(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void mulf64x8(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void divf64x8(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result);
void sqrtf64x8(const double *HWY_RESTRICT a, double *HWY_RESTRICT result);
void fmaf64x8(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              const double *HWY_RESTRICT c, double *HWY_RESTRICT result);

/* IEEE-754 binary32 x16 */
void addf32x16(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
               float *HWY_RESTRICT result);
void subf32x16(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
               float *HWY_RESTRICT result);
void mulf32x16(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
               float *HWY_RESTRICT result);
void divf32x16(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
               float *HWY_RESTRICT result);
void sqrtf32x16(const float *HWY_RESTRICT a, float *HWY_RESTRICT result);
void fmaf32x16(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
               const float *HWY_RESTRICT c, float *HWY_RESTRICT result);

/* 1024-bits */

/* IEEE-754 binary64 x16 */
void addf64x16(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
               double *HWY_RESTRICT result);
void subf64x16(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
               double *HWY_RESTRICT result);
void mulf64x16(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
               double *HWY_RESTRICT result);
void divf64x16(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
               double *HWY_RESTRICT result);
void sqrtf64x16(const double *HWY_RESTRICT a, double *HWY_RESTRICT result);
void fmaf64x16(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
               const double *HWY_RESTRICT c, double *HWY_RESTRICT result);

} // namespace dynamic_dispatch

namespace static_dispatch {

/* Single vector functions static dispatch */

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
f32x2_v addf32x2(const f32x2_v a, const f32x2_v b);
f32x2_v subf32x2(const f32x2_v a, const f32x2_v b);
f32x2_v mulf32x2(const f32x2_v a, const f32x2_v b);
f32x2_v divf32x2(const f32x2_v a, const f32x2_v b);
f32x2_v sqrtf32x2(const f32x2_v a);
f32x2_v fmaf32x2(const f32x2_v a, const f32x2_v b, const f32x2_v c);
#endif

/* 128-bits */
#if HWY_MAX_BYTES >= 16
f64x2_v addf64x2(const f64x2_v a, const f64x2_v b);
f64x2_v subf64x2(const f64x2_v a, const f64x2_v b);
f64x2_v mulf64x2(const f64x2_v a, const f64x2_v b);
f64x2_v divf64x2(const f64x2_v a, const f64x2_v b);
f64x2_v sqrtf64x2(const f64x2_v a);
f64x2_v fmaf64x2(const f64x2_v a, const f64x2_v b, const f64x2_v c);

f32x4_v addf32x4(const f32x4_v a, const f32x4_v b);
f32x4_v subf32x4(const f32x4_v a, const f32x4_v b);
f32x4_v mulf32x4(const f32x4_v a, const f32x4_v b);
f32x4_v divf32x4(const f32x4_v a, const f32x4_v b);
f32x4_v sqrtf32x4(const f32x4_v a);
f32x4_v fmaf32x4(const f32x4_v a, const f32x4_v b, const f32x4_v c);
#endif

/* 256-bits */
#if HWY_MAX_BYTES >= 32
f64x4_v addf64x4(const f64x4_v a, const f64x4_v b);
f64x4_v subf64x4(const f64x4_v a, const f64x4_v b);
f64x4_v mulf64x4(const f64x4_v a, const f64x4_v b);
f64x4_v divf64x4(const f64x4_v a, const f64x4_v b);
f64x4_v sqrtf64x4(const f64x4_v a);
f64x4_v fmaf64x4(const f64x4_v a, const f64x4_v b, const f64x4_v c);

f32x8_v addf32x8(const f32x8_v a, const f32x8_v b);
f32x8_v subf32x8(const f32x8_v a, const f32x8_v b);
f32x8_v mulf32x8(const f32x8_v a, const f32x8_v b);
f32x8_v divf32x8(const f32x8_v a, const f32x8_v b);
f32x8_v sqrtf32x8(const f32x8_v a);
f32x8_v fmaf32x8(const f32x8_v a, const f32x8_v b, const f32x8_v c);
#endif

/* 512-bits */
#if HWY_MAX_BYTES >= 64
f64x8_v addf64x8(const f64x8_v a, const f64x8_v b);
f64x8_v subf64x8(const f64x8_v a, const f64x8_v b);
f64x8_v mulf64x8(const f64x8_v a, const f64x8_v b);
f64x8_v divf64x8(const f64x8_v a, const f64x8_v b);
f64x8_v sqrtf64x8(const f64x8_v a);
f64x8_v fmaf64x8(const f64x8_v a, const f64x8_v b, const f64x8_v c);

f32x16_v addf32x16(const f32x16_v a, const f32x16_v b);
f32x16_v subf32x16(const f32x16_v a, const f32x16_v b);
f32x16_v mulf32x16(const f32x16_v a, const f32x16_v b);
f32x16_v divf32x16(const f32x16_v a, const f32x16_v b);
f32x16_v sqrtf32x16(const f32x16_v a);
f32x16_v fmaf32x16(const f32x16_v a, const f32x16_v b, const f32x16_v c);
#endif

/* 1024-bits */
#if HWY_MAX_BYTES >= 128
f64x16_v addf64x16(const f64x16_v a, const f64x16_v b);
f64x16_v subf64x16(const f64x16_v a, const f64x16_v b);
f64x16_v mulf64x16(const f64x16_v a, const f64x16_v b);
f64x16_v divf64x16(const f64x16_v a, const f64x16_v b);
f64x16_v sqrtf64x16(const f64x16_v a);
f64x16_v fmaf64x16(const f64x16_v a, const f64x16_v b, const f64x16_v c);
#endif

} // namespace static_dispatch

} // namespace prism::sr::vector

#endif // __PRISM_SR_HW_H__