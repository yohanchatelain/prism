
#include "src/debug.h"

// Generates code for every target that this compiler can support.
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/ud_vector_dynamic.cpp" // this file
#include "hwy/foreach_target.h"                // must come before highway.h

// clang-format off
#include "hwy/highway.h"
#include "src/ud_vector-inl.h"
#include "src/ud_vector.h"
// clang-format on


// Optional, can instead add HWY_ATTR to all functions.
HWY_BEFORE_NAMESPACE();
namespace prism::ud::vector::dynamic_dispatch {

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;

// print target
#if defined PRISM_DEBUG_TARGET
#define STR_HELPER(x) #x
#define XSTR(x) STR_HELPER(x)
#pragma message("HWY_TARGET: " XSTR(HWY_TARGET_STR))
#endif

/* for dynamic dispatch */

template <typename T>
HWY_INLINE void _round(const T *HWY_RESTRICT a, T *HWY_RESTRICT result,
                       const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto res = round(d, a_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

// TODO: make two versions: one fixed size and one scalable
template <typename T>
void _add(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
          T *HWY_RESTRICT result, const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto b_vec = hn::LoadN(d, b + i, count - i);
    auto res = add(d, a_vec, b_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

template <typename T>
void _sub(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
          T *HWY_RESTRICT result, const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto b_vec = hn::LoadN(d, b + i, count - i);
    auto res = sub(d, a_vec, b_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

template <typename T>
void _mul(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
          T *HWY_RESTRICT result, const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto b_vec = hn::LoadN(d, b + i, count - i);
    auto res = mul(d, a_vec, b_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

template <typename T>
void _div(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
          T *HWY_RESTRICT result, const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto b_vec = hn::LoadN(d, b + i, count - i);
    auto res = div(d, a_vec, b_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

template <typename T>
void _sqrt(const T *HWY_RESTRICT a, T *HWY_RESTRICT result,
           const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto res = sqrt(d, a_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

template <typename T>
void _fma(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
          const T *HWY_RESTRICT c, T *HWY_RESTRICT result, const size_t count) {
  using D = hn::ScalableTag<T>;
  const D d{};
  const size_t N = hn::Lanes(d);

  for (size_t i = 0; i <= count; i += N) {
    auto a_vec = hn::LoadN(d, a + i, count - i);
    auto b_vec = hn::LoadN(d, b + i, count - i);
    auto c_vec = hn::LoadN(d, b + i, count - i);
    auto res = fma(d, a_vec, b_vec, c_vec);
    hn::StoreN(res, d, result + i, count - i);
  }
}

/* binary32 */

void _round_f32(const float *HWY_RESTRICT a, float *HWY_RESTRICT result,
                const size_t count) {
  _round(a, result, count);
}

void _add_f32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result, const size_t count) {
  _add(a, b, result, count);
}
void _sub_f32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result, const size_t count) {
  _sub(a, b, result, count);
}

void _mul_f32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result, const size_t count) {
  _mul(a, b, result, count);
}

void _div_f32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              float *HWY_RESTRICT result, const size_t count) {
  _div(a, b, result, count);
}

void _fma_f32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
              const float *HWY_RESTRICT c, float *HWY_RESTRICT result,
              const size_t count) {
  _fma(a, b, c, result, count);
}

/* binary64 */

void _sqrt_f32(const float *HWY_RESTRICT a, float *HWY_RESTRICT result,
               const size_t count) {
  _sqrt(a, result, count);
}

void _round_f64(const double *HWY_RESTRICT a,  double *HWY_RESTRICT result,
                const size_t count) {
  _round(a, result, count);
}


void _add_f64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result, const size_t count) {
  _add(a, b, result, count);
}

void _sub_f64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result, const size_t count) {
  _sub(a, b, result, count);
}

void _mul_f64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result, const size_t count) {
  _mul(a, b, result, count);
}

void _div_f64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              double *HWY_RESTRICT result, const size_t count) {
  _div(a, b, result, count);
}

void _sqrt_f64(const double *HWY_RESTRICT a, double *HWY_RESTRICT result,
               const size_t count) {
  _sqrt(a, result, count);
}

void _fma_f64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
              const double *HWY_RESTRICT c, double *HWY_RESTRICT result,
              const size_t count) {
  _fma(a, b, c, result, count);
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace prism::ud::vector::dynamic_dispatch
HWY_AFTER_NAMESPACE();



#if HWY_ONCE

namespace prism::ud::vector::dynamic_dispatch {

namespace {

HWY_EXPORT(_round_f32);
HWY_EXPORT(_add_f32);
HWY_EXPORT(_sub_f32);
HWY_EXPORT(_mul_f32);
HWY_EXPORT(_div_f32);
HWY_EXPORT(_sqrt_f32);
HWY_EXPORT(_fma_f32);

HWY_EXPORT(_round_f64);
HWY_EXPORT(_add_f64);
HWY_EXPORT(_sub_f64);
HWY_EXPORT(_mul_f64);
HWY_EXPORT(_div_f64);
HWY_EXPORT(_sqrt_f64);
HWY_EXPORT(_fma_f64);

}  // namespace

/* Array functions */

/* binary32 */
void round(const float *HWY_RESTRICT a,
           float *HWY_RESTRICT result, const size_t count) {
    return HWY_DYNAMIC_DISPATCH(_round_f32)(a, result, count);
}

void addf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count) {
 return HWY_DYNAMIC_DISPATCH(_add_f32)(a, b, result, count);
}

void subf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_sub_f32)(a, b, result, count);
}

void mulf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_mul_f32)(a, b, result, count);
}

void divf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            float *HWY_RESTRICT result, const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_div_f32)(a, b, result, count);
}

void sqrtf32(const float *HWY_RESTRICT a, float *HWY_RESTRICT result,
             const size_t count) {
 return HWY_DYNAMIC_DISPATCH(_sqrt_f32)(a, result, count);
}

void fmaf32(const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,
            const float *HWY_RESTRICT c, float *HWY_RESTRICT result,
            const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_fma_f32)(a, b, c, result, count);
}

/* binary64 */
void round(const double *HWY_RESTRICT a,
           double *HWY_RESTRICT result, const size_t count) {
    return HWY_DYNAMIC_DISPATCH(_round_f64)(a, result, count);
}

void addf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_add_f64)(a, b, result, count);
}

void subf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count) {
   return HWY_DYNAMIC_DISPATCH(_sub_f64)(a, b, result, count);
}

void mulf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count) {
    return HWY_DYNAMIC_DISPATCH(_mul_f64)(a, b, result, count);
}

void divf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            double *HWY_RESTRICT result, const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_div_f64)(a, b, result, count);
}

void sqrtf64(const double *HWY_RESTRICT a, double *HWY_RESTRICT result,
             const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_sqrt_f64)(a, result, count);
}

void fmaf64(const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,
            const double *HWY_RESTRICT c, double *HWY_RESTRICT result,
            const size_t count) {
  return HWY_DYNAMIC_DISPATCH(_fma_f64)(a, b, c, result, count);
}

/* Single vector instructions with dynamic dispatch */

#define define_array_unary_op_dynamic(name, count)                             \
  void name##f32##x##count(const float *HWY_RESTRICT a,              \
                                     float *HWY_RESTRICT result) {             \
    return name##f32(a, result, count);                                      \
  }                                                                            \
  void name##f64x##count(const double *HWY_RESTRICT a,               \
                                   double *HWY_RESTRICT result) {              \
    return name##f64(a, result, count);                                     \
  }

#define define_array_bin_op_dynamic(name, count)                               \
  void name##f32##x##count(const float *HWY_RESTRICT a,              \
                                     const float *HWY_RESTRICT b,              \
                                     float *HWY_RESTRICT result) {             \
    return name##f32(a, b, result, count);                                   \
  }                                                                            \
  void name##f64x##count(const double *HWY_RESTRICT a,               \
                                   const double *HWY_RESTRICT b,               \
                                   double *HWY_RESTRICT result) {              \
    return name##f64(a, b, result, count);                                  \
  }

#define define_array_ter_op_dynamic(name, count)                               \
  void name##f32##x##count(                                          \
      const float *HWY_RESTRICT a, const float *HWY_RESTRICT b,                \
      const float *HWY_RESTRICT c, float *HWY_RESTRICT result) {               \
    return name##f32(a, b, c, result, count);                                \
  }                                                                            \
  void name##f64x##count(                                            \
      const double *HWY_RESTRICT a, const double *HWY_RESTRICT b,              \
      const double *HWY_RESTRICT c, double *HWY_RESTRICT result) {             \
    return name##f64(a, b, c, result, count);                               \
  }

/* 64-bits */
define_array_bin_op_dynamic(add, 2);
define_array_bin_op_dynamic(sub, 2);
define_array_bin_op_dynamic(mul, 2);
define_array_bin_op_dynamic(div, 2);
define_array_unary_op_dynamic(sqrt, 2);
define_array_ter_op_dynamic(fma, 2);

/* 128-bits */
define_array_bin_op_dynamic(add, 4);
define_array_bin_op_dynamic(sub, 4);
define_array_bin_op_dynamic(mul, 4);
define_array_bin_op_dynamic(div, 4);
define_array_unary_op_dynamic(sqrt, 4);
define_array_ter_op_dynamic(fma, 4);

/* 256-bits */
define_array_bin_op_dynamic(add, 8);
define_array_bin_op_dynamic(sub, 8);
define_array_bin_op_dynamic(mul, 8);
define_array_bin_op_dynamic(div, 8);
define_array_unary_op_dynamic(sqrt, 8);
define_array_ter_op_dynamic(fma, 8);

/* 512-bits */
define_array_bin_op_dynamic(add, 16);
define_array_bin_op_dynamic(sub, 16);
define_array_bin_op_dynamic(mul, 16);
define_array_bin_op_dynamic(div, 16);
define_array_unary_op_dynamic(sqrt, 16);
define_array_ter_op_dynamic(fma, 16);

/* 1024-bits */
define_array_bin_op_dynamic(add, 32);
define_array_bin_op_dynamic(sub, 32);
define_array_bin_op_dynamic(mul, 32);
define_array_bin_op_dynamic(div, 32);
define_array_unary_op_dynamic(sqrt, 32);
define_array_ter_op_dynamic(fma, 32);

} // namespace prism::ud::vector::dynamic_dispatch

#endif // HWY_ONCE