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

#include "prism_api.h"
#include "sr_vector.h"
#include "ud_vector.h"
#include "utils.h"
#include "xoshiro.h"

extern "C" {

void interflop_prism_set_default_virtual_precision_binary32(int32_t t) {
  prism::sr::set_default_virtual_precision<float>(t);
}

void interflop_prism_set_default_virtual_precision_binary64(int32_t t) {
  prism::sr::set_default_virtual_precision<double>(t);
}

int32_t interflop_prism_get_default_virtual_precision_binary32(void) {
  return prism::sr::default_virtual_precision_f32.load(
      std::memory_order_relaxed);
}

int32_t interflop_prism_get_default_virtual_precision_binary64(void) {
  return prism::sr::default_virtual_precision_f64.load(
      std::memory_order_relaxed);
}

int32_t interflop_prism_get_virtual_precision_binary32(void) {
  return prism::sr::get_virtual_precision<float>();
}

int32_t interflop_prism_get_virtual_precision_binary64(void) {
  return prism::sr::get_virtual_precision<double>();
}

void interflop_prism_set_thread_virtual_precision_binary32(int32_t t) {
  prism::sr::set_virtual_precision<float>(t);
}

void interflop_prism_set_thread_virtual_precision_binary64(int32_t t) {
  prism::sr::set_virtual_precision<double>(t);
}

uint64_t interflop_prism_get_seed(void) { return get_user_seed(); }

void interflop_prism_set_seed(uint64_t seed) { set_user_seed(seed); }

void interflop_prism_set_rounding_mode(int32_t mode) {
  prism::sr::set_default_rounding_mode(mode);
}

int32_t interflop_prism_get_rounding_mode(void) {
  return prism::sr::get_rounding_mode();
}

void interflop_prism_set_thread_rounding_mode(int32_t mode) {
  prism::sr::set_rounding_mode(mode);
}

/* =========================================================================
 * Array Interface Implementations
 * ========================================================================= */

/* Stochastic Rounding (SR) Array Operations - binary32 */
void interflop_prism_sr_round_f32(const float *sigma, const float *tau,
                                 float *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::round(sigma, tau, result, count);
}

void interflop_prism_sr_add_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::addf32(a, b, result, count);
}

void interflop_prism_sr_sub_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::subf32(a, b, result, count);
}

void interflop_prism_sr_mul_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::mulf32(a, b, result, count);
}

void interflop_prism_sr_div_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::divf32(a, b, result, count);
}

void interflop_prism_sr_sqrt_f32(const float *a, float *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::sqrtf32(a, result, count);
}

void interflop_prism_sr_fma_f32(const float *a, const float *b, const float *c,
                               float *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::fmaf32(a, b, c, result, count);
}

/* Stochastic Rounding (SR) Array Operations - binary64 */
void interflop_prism_sr_round_f64(const double *sigma, const double *tau,
                                 double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::round(sigma, tau, result, count);
}

void interflop_prism_sr_add_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::addf64(a, b, result, count);
}

void interflop_prism_sr_sub_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::subf64(a, b, result, count);
}

void interflop_prism_sr_mul_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::mulf64(a, b, result, count);
}

void interflop_prism_sr_div_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::divf64(a, b, result, count);
}

void interflop_prism_sr_sqrt_f64(const double *a, double *result,
                                size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::sqrtf64(a, result, count);
}

void interflop_prism_sr_fma_f64(const double *a, const double *b,
                               const double *c, double *result, size_t count) {
  prism::sr::vector::PRISM_DISPATCH::variable::fmaf64(a, b, c, result, count);
}

/* Up-Down Rounding (UD) Array Operations - binary32 */
void interflop_prism_ud_round_f32(const float *a, float *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::round(a, result, count);
}

void interflop_prism_ud_add_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::addf32(a, b, result, count);
}

void interflop_prism_ud_sub_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::subf32(a, b, result, count);
}

void interflop_prism_ud_mul_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::mulf32(a, b, result, count);
}

void interflop_prism_ud_div_f32(const float *a, const float *b, float *result,
                               size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::divf32(a, b, result, count);
}

void interflop_prism_ud_sqrt_f32(const float *a, float *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::sqrtf32(a, result, count);
}

void interflop_prism_ud_fma_f32(const float *a, const float *b, const float *c,
                               float *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::fmaf32(a, b, c, result, count);
}

/* Up-Down Rounding (UD) Array Operations - binary64 */
void interflop_prism_ud_round_f64(const double *a, double *result,
                                 size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::round(a, result, count);
}

void interflop_prism_ud_add_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::addf64(a, b, result, count);
}

void interflop_prism_ud_sub_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::subf64(a, b, result, count);
}

void interflop_prism_ud_mul_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::mulf64(a, b, result, count);
}

void interflop_prism_ud_div_f64(const double *a, const double *b,
                               double *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::divf64(a, b, result, count);
}

void interflop_prism_ud_sqrt_f64(const double *a, double *result,
                                size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::sqrtf64(a, result, count);
}

void interflop_prism_ud_fma_f64(const double *a, const double *b,
                               const double *c, double *result, size_t count) {
  prism::ud::vector::PRISM_DISPATCH::variable::fmaf64(a, b, c, result, count);
}

} // extern "C"
