
#include <cstdint>
#include <stdlib.h>

#ifndef __PRISM_DEBUG_VECTOR_INL_H_
#define __PRISM_DEBUG_VECTOR_INL_H_

static auto __attribute__((noinline)) __attribute__((unused))
prism_print_debug() -> bool {
#ifdef PRISM_DEBUG
  static int _print_debug = -1;
  if (_print_debug == -1) {
    const char *env = "PRISM_DEBUG";
    const char *env_debug = getenv(env);
    _print_debug = env_debug && env_debug[0] == '1';
  }
  return static_cast<bool>(_print_debug);
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

#include "hwy/highway.h"
#include "hwy/print-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

HWY_API void debug_msg(const char *msg) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug()) {
    return;
  }
  fprintf(stderr, "%s\n", msg);
#endif
}

template <typename T> auto _get_format_string(const bool hex) -> const char * {
  if constexpr (std::is_same<T, float>::value) {
    return hex ? " %+.6a" : " %+.7e";
  } else if constexpr (std::is_same<T, double>::value) {
    return hex ? " %+.13a" : " %+.17e";
  } else if constexpr (std::is_same<T, std::int32_t>::value) {
    return hex ? " %08x" : " %d";
  } else if constexpr (std::is_same<T, std::uint32_t>::value) {
    return hex ? " %08x" : " %u";
  } else if constexpr (std::is_same<T, std::int64_t>::value) {
    return hex ? " %016llx" : " %lld";
  } else if constexpr (std::is_same<T, std::uint64_t>::value) {
    return hex ? " %016llx" : " %llu";
  } else {
    return ""; // Default case
  }
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_API void debug_vec(const D d, const char *msg, const V &a,
                       const bool hex = true) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug()) {
    return;
  }
  const char *format = _get_format_string<T>(hex);
  const auto N = hn::Lanes(d);
  hn::Print(d, msg, a, 0, N, format);
#endif
}

template <class D, class M, typename T = hn::TFromD<D>>
HWY_API void debug_mask(const D d, const char *msg, M a) {
#ifdef PRISM_DEBUG
  if (not prism_print_debug()) {
    return;
  }
  using DU = hn::RebindToUnsigned<D>;
  const DU du{};
  using U = hn::TFromD<DU>;
  const char *format = _get_format_string<U>(false);
  const auto N = hn::Lanes(d);
  const auto mu = hn::RebindMask(du, a);
  const auto ma = hn::VecFromMask(du, mu);
  hn::Print(du, msg, ma, 0, N, format);
#endif
}

#define DISABLED_DEBUG_VEC                                                     \
  template <typename... Args> constexpr void debug_vec(Args &&...args) {};

#define DISABLED_DEBUG_MSG                                                     \
  template <typename... Args> constexpr void debug_msg(Args &&...args) {};

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace prism::vector::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#endif // PRISM_DEBUG_VECTOR_INL_H_
