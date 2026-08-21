/*****************************************************************************\
 *                                                                           *\
 *  This file is part of the Verificarlo project,                            *\
 *  under the Apache License v2.0 with LLVM Exceptions.                      *\
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.                 *\
 *  See https://llvm.org/LICENSE.txt for license information.                *\
 *                                                                           *\
 *  Copyright (c) 2026                                                       *\
 *     Verificarlo Contributors                                              *\
 *                                                                           *\
 ****************************************************************************/

#ifndef __PRISM_API_H__
#define __PRISM_API_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Process-wide setters: the new value reaches every thread, including threads
 * that have already executed instrumented arithmetic, on their next operation.
 * A process-wide set discards any outstanding per-thread override. */
void interflop_prism_set_default_virtual_precision_binary32(int32_t t);
void interflop_prism_set_default_virtual_precision_binary64(int32_t t);

int32_t interflop_prism_get_default_virtual_precision_binary32(void);
int32_t interflop_prism_get_default_virtual_precision_binary64(void);

/* Precision the calling thread will actually round at. Use this, not the
 * default getter, to check that a setting took effect. */
int32_t interflop_prism_get_virtual_precision_binary32(void);
int32_t interflop_prism_get_virtual_precision_binary64(void);

/* Per-thread override, in effect until the next process-wide set. */
void interflop_prism_set_thread_virtual_precision_binary32(int32_t t);
void interflop_prism_set_thread_virtual_precision_binary64(int32_t t);

uint64_t interflop_prism_get_seed(void);
void interflop_prism_set_seed(uint64_t seed);

/* Rounding modes */
#define INTERFLOP_PRISM_SR 0
#define INTERFLOP_PRISM_RN 1

/* Process-wide, like the precision setter above. */
void interflop_prism_set_rounding_mode(int32_t mode);
/* Mode the calling thread will actually round with. */
int32_t interflop_prism_get_rounding_mode(void);
/* Per-thread override, in effect until the next process-wide set. */
void interflop_prism_set_thread_rounding_mode(int32_t mode);

/* =========================================================================
 * Array Interface (Contiguous memory buffers)
 * ========================================================================= */

/* Stochastic Rounding (SR) Array Operations - binary32 (float) */
void interflop_prism_sr_round_f32(const float *sigma, const float *tau,
                                 float *result, size_t count);
void interflop_prism_sr_add_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_sr_sub_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_sr_mul_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_sr_div_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_sr_sqrt_f32(const float *a, float *result, size_t count);
void interflop_prism_sr_fma_f32(const float *a, const float *b, const float *c,
                               float *result, size_t count);

/* Stochastic Rounding (SR) Array Operations - binary64 (double) */
void interflop_prism_sr_round_f64(const double *sigma, const double *tau,
                                 double *result, size_t count);
void interflop_prism_sr_add_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_sr_sub_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_sr_mul_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_sr_div_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_sr_sqrt_f64(const double *a, double *result, size_t count);
void interflop_prism_sr_fma_f64(const double *a, const double *b,
                               const double *c, double *result, size_t count);

/* Up-Down Rounding (UD) Array Operations - binary32 (float) */
void interflop_prism_ud_round_f32(const float *a, float *result, size_t count);
void interflop_prism_ud_add_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_ud_sub_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_ud_mul_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_ud_div_f32(const float *a, const float *b, float *result,
                               size_t count);
void interflop_prism_ud_sqrt_f32(const float *a, float *result, size_t count);
void interflop_prism_ud_fma_f32(const float *a, const float *b, const float *c,
                               float *result, size_t count);

/* Up-Down Rounding (UD) Array Operations - binary64 (double) */
void interflop_prism_ud_round_f64(const double *a, double *result,
                                 size_t count);
void interflop_prism_ud_add_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_ud_sub_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_ud_mul_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_ud_div_f64(const double *a, const double *b,
                               double *result, size_t count);
void interflop_prism_ud_sqrt_f64(const double *a, double *result, size_t count);
void interflop_prism_ud_fma_f64(const double *a, const double *b,
                               const double *c, double *result, size_t count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __PRISM_API_H__
