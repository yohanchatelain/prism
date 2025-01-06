
#include <cstdint>
#include <stdlib.h>

#ifndef __PRISM_DEBUG_VECTOR_INL_H_
#define __PRISM_DEBUG_VECTOR_INL_H_

static bool __attribute__((noinline)) __attribute__((unused))
prism_print_debug() {
#ifdef PRISM_DEBUG
  static int _print_debug = -1;
  if (_print_debug == -1) {
    const char *HWY_RESTRICT env = "PRISM_DEBUG";
    const char *HWY_RESTRICT env_debug = getenv(env);
    _print_debug = env_debug && env_debug[0] == '1';
  }
  return (bool)_print_debug;
#else
  return false;
#endif
}

#endif // __PRISM_DEBUG_VECTOR_INL_H_

#if defined(PRISM_DEBUG_VECTOR_INL_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_DEBUG_VECTOR_INL_H_
#undef PRISM_DEBUG_VECTOR_INL_H_
#else
#define PRISM_DEBUG_VECTOR_INL_H_
#endif

#include "hwy/aligned_allocator.h"
#include "hwy/highway.h"
#include "hwy/print-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism {

namespace vector {

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

HWY_API void debug_msg(const char *HWY_RESTRICT msg) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug())
    return;
  alignas(HWY_ALIGNMENT) const char *HWY_RESTRICT _aligned_msg = msg;
  printf("%s\n", _aligned_msg);
#endif
}

template <typename T> const char *_get_format_string(const bool hex) {
  if constexpr (std::is_same<T, float>::value) {
    return hex ? " %+.6a" : " %+.7e";
  } else if constexpr (std::is_same<T, double>::value) {
    return hex ? " %+.13a" : " %+.17e";
  } else if constexpr (std::is_same<T, std::int32_t>::value) {
    return hex ? " %08x" : " %d";
  } else if constexpr (std::is_same<T, std::int64_t>::value) {
    return hex ? " %016x" : " %lld";
  } else if constexpr (std::is_same<T, std::uint32_t>::value) {
    return hex ? " %08x" : " %u";
  } else if constexpr (std::is_same<T, std::uint64_t>::value) {
    return hex ? " %016x" : " %llu";
  } else {
    return ""; // Default case
  }
}

template <class D, class V, typename T = hn::TFromD<D>>
HWY_API void debug_vec(const D d, const char *HWY_RESTRICT msg, const V &a,
                       const bool hex = true) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug())
    return;
  const char *HWY_RESTRICT format = _get_format_string<T>(hex);
  const size_t N = hn::Lanes(d);
  hn::Print(d, msg, a, 0, N - 1, format);
#endif
}

template <class D, class M, typename T = hn::TFromD<D>>
HWY_API void debug_mask(const D d, const char *HWY_RESTRICT msg, const M &a) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug())
    return;
  debug_vec(d, msg, hn::VecFromMask(d, a));
#endif
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace vector
} // namespace prism
HWY_AFTER_NAMESPACE();

#endif // PRISM_DEBUG_VECTOR_INL_H_
