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

// Process-wide PRISM state, built into libprism-config. Every other PRISM
// library uses it through the declarations in utils.h and seed.h, so the
// libraries share one configuration even when they do not share a symbol
// scope (verificarlo/prism#24).

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <random>

#include "src/seed.h"
#include "src/utils.h"

namespace prism::sr {

std::atomic<int32_t> default_virtual_precision_f32{
    utils::IEEE754<float>::precision};
std::atomic<int32_t> default_virtual_precision_f64{
    utils::IEEE754<double>::precision};
std::atomic<int32_t> default_rounding_mode{PRISM_SR};

std::atomic<uint32_t> config_epoch{kEpochNeverObserved + 1};

// The initial values are placeholders: observed_epoch differs from
// config_epoch, so a thread loads the defaults before it first reads them.
PRISM_CONSTINIT thread_local int32_t virtual_precision_f32 =
    utils::IEEE754<float>::precision;
PRISM_CONSTINIT thread_local int32_t virtual_precision_f64 =
    utils::IEEE754<double>::precision;
PRISM_CONSTINIT thread_local int32_t rounding_mode = PRISM_SR;
PRISM_CONSTINIT thread_local uint32_t observed_epoch = kEpochNeverObserved;

} // namespace prism::sr

auto get_thread_id() -> std::uint64_t {
  static std::atomic<std::uint64_t> thread_counter{0};
  thread_local std::uint64_t tid =
      thread_counter.fetch_add(1, std::memory_order_relaxed);
  return tid;
}

auto seed_state(bool set, std::uint64_t new_seed) -> std::uint64_t {
  static bool initialized = false;
  static std::uint64_t seed = 0;
  if (set) {
    seed = new_seed;
    initialized = true;
  } else if (not initialized) {
    const char *seed_str = getenv("PRISM_SEED");
    if (seed_str != nullptr) {
      char *endptr = nullptr;
      seed = strtoll(seed_str, &endptr, 10);
      if (*endptr != '\0') {
        seed = 0;
      }
    } else {
      std::random_device rd;
      seed = rd();
    }
    initialized = true;
  }
  return seed;
}
