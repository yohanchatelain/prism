#include <cstdint>
#include <iostream>

#if defined(PRISM_DEBUG_VECTOR_INL_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_DEBUG_VECTOR_INL_H_
#undef PRISM_DEBUG_VECTOR_INL_H_
#else
#define PRISM_DEBUG_VECTOR_INL_H_
#endif

#include "hwy/highway.h"
#include "hwy/print-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism {

namespace vector {

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

HWY_API bool _print_debug() {
#ifdef PRISM_DEBUG
  const char *env_debug = getenv("PRISM_DEBUG");
  return env_debug && std::string(env_debug) == "1";
#else
  return false;
#endif
}

HWY_API void debug_msg(const std::string &msg) {
#ifdef PRISM_DEBUG
  if (not _print_debug())
    return;
  std::cout << msg << std::endl;
#endif
}

template <class D, class V, typename T = hn::TFromD<D>>
HWY_API void debug_vec(const std::string &msg, const V &a,
                       const bool hex = true) {
#ifdef PRISM_DEBUG
  const D d;
  if (not _print_debug())
    return;
  if (std::is_same<T, float>::value) {
    if (hex) {
      hn::Print(d, msg.c_str(), a, 0, 15, " %+.6a");
    } else {
      hn::Print(d, msg.c_str(), a, 0, 15, " %+.7e");
    }
  } else if (std::is_same<T, double>::value) {
    if (hex) {
      hn::Print(d, msg.c_str(), a, 0, 15, " %+.13a");
    } else {
      hn::Print(d, msg.c_str(), a, 0, 15, " %+.17e");
    }
  } else if (std::is_same<T, std::int32_t>::value) {
    if (hex) {
      hn::Print(d, msg.c_str(), a, 0, 15, " %08x");
    } else {
      hn::Print(d, msg.c_str(), a);
    }
  } else if (std::is_same<T, std::int64_t>::value) {
    if (hex) {
      hn::Print(d, msg.c_str(), a, 0, 15, " %016x");
    } else {
      hn::Print(d, msg.c_str(), a);
    }
  } else {
    hn::Print(d, msg.c_str(), a);
  }
#endif
}

template <class D, class M, typename T = hn::TFromD<D>>
HWY_API void debug_mask(const std::string &msg, const M &a) {
#ifdef PRISM_DEBUG
  const D d;
  if (not _print_debug())
    return;
  debug_vec<D>(msg, hn::VecFromMask(d, a));
#endif
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace vector
} // namespace prism
HWY_AFTER_NAMESPACE();

#endif // PRISM_DEBUG_VECTOR_INL_H_