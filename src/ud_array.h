/*****************************************************************************\
 *                                                                           *
 *  This file is part of the Verificarlo project,                            *
 *  under the Apache License v2.0 with LLVM Exceptions.                      *
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                 *
 *  See https://llvm.org/LICENSE.txt for license information.                *
 *                                                                           *
 *  Copyright (c) 2026                                                       *
 *     Verificarlo Contributors                                              *
 *                                                                           *
 ****************************************************************************/

#ifndef __PRISM_UD_ARRAY_H__
#define __PRISM_UD_ARRAY_H__

#include <cstddef>

namespace prism::ud::array {

namespace dynamic_dispatch {

/* IEEE-754 binary32 */
void round(const float *a, float *result, size_t count);
void addf32(const float *a, const float *b, float *result, size_t count);
void subf32(const float *a, const float *b, float *result, size_t count);
void mulf32(const float *a, const float *b, float *result, size_t count);
void divf32(const float *a, const float *b, float *result, size_t count);
void sqrtf32(const float *a, float *result, size_t count);
void fmaf32(const float *a, const float *b, const float *c, float *result,
            size_t count);

/* IEEE-754 binary64 */
void round(const double *a, double *result, size_t count);
void addf64(const double *a, const double *b, double *result, size_t count);
void subf64(const double *a, const double *b, double *result, size_t count);
void mulf64(const double *a, const double *b, double *result, size_t count);
void divf64(const double *a, const double *b, double *result, size_t count);
void sqrtf64(const double *a, double *result, size_t count);
void fmaf64(const double *a, const double *b, const double *c, double *result,
            size_t count);

} // namespace dynamic_dispatch

namespace static_dispatch {

/* IEEE-754 binary32 */
void round(const float *a, float *result, size_t count);
void addf32(const float *a, const float *b, float *result, size_t count);
void subf32(const float *a, const float *b, float *result, size_t count);
void mulf32(const float *a, const float *b, float *result, size_t count);
void divf32(const float *a, const float *b, float *result, size_t count);
void sqrtf32(const float *a, float *result, size_t count);
void fmaf32(const float *a, const float *b, const float *c, float *result,
            size_t count);

/* IEEE-754 binary64 */
void round(const double *a, double *result, size_t count);
void addf64(const double *a, const double *b, double *result, size_t count);
void subf64(const double *a, const double *b, double *result, size_t count);
void mulf64(const double *a, const double *b, double *result, size_t count);
void divf64(const double *a, const double *b, double *result, size_t count);
void sqrtf64(const double *a, double *result, size_t count);
void fmaf64(const double *a, const double *b, const double *c, double *result,
            size_t count);

} // namespace static_dispatch

} // namespace prism::ud::array

#endif // __PRISM_UD_ARRAY_H__
