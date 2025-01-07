
#include "src/debug.h"

// Generates code for every target that this compiler can support.
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "src/sr_vector_static.cpp" // this file
#include "hwy/foreach_target.h"                // must come before highway.h

// clang-format off
#include "hwy/highway.h"
#include "src/sr_vector-inl.h"
#include "src/sr_vector.h"
#include "src/debug_vector-inl.h"
// clang-format on

// Optional, can instead add HWY_ATTR to all functions.
HWY_BEFORE_NAMESPACE();
namespace prism::sr::vector::static_dispatch {

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;

// print target
#if defined PRISM_DEBUG_TARGET
#define STR_HELPER(x) #x
#define XSTR(x) STR_HELPER(x)
#pragma message("HWY_TARGET: " XSTR(HWY_TARGET_STR))
#endif


template <typename T, std::size_t N>
void _print(const T *HWY_RESTRICT a, const char *msg) {
  std::cout << msg;
  for (int i = 0; i < N; i++) {
    std::cout << a[i] << " ";
  }
  std::cout << std::endl;
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _addxN(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
            T *HWY_RESTRICT result) {
  const D d{};
  const auto va = hn::Load(d, a);
  const auto vb = hn::Load(d, b);
  auto res = add(d, va, vb);
  hn::Store(res, d, result);
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _subxN(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
            T *HWY_RESTRICT result) {
  const D d;
  const auto va = hn::Load(d, a);
  const auto vb = hn::Load(d, b);
  auto res = sub(d, va, vb);
  hn::Store(res, d, result);
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _mulxN(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
            T *HWY_RESTRICT result) {
  const D d;
  const auto va = hn::Load(d, a);
  const auto vb = hn::Load(d, b);
  auto res = mul(d, va, vb);
  hn::Store(res, d, result);
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _divxN(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
            T *HWY_RESTRICT result) {
  const D d;
  const auto va = hn::Load(d, a);
  const auto vb = hn::Load(d, b);
  auto res = div(d, va, vb);
  hn::Store(res, d, result);
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _sqrtxN(const T *HWY_RESTRICT a, T *HWY_RESTRICT result) {
  const D d;
  const auto va = hn::Load(d, a);
  auto res = sqrt(d, va);
  hn::Store(res, d, result);
}

template <typename T, std::size_t N, class D = hn::FixedTag<T, N>,
          class V = hn::Vec<D>>
void _fmaxN(const T *HWY_RESTRICT a, const T *HWY_RESTRICT b,
            const T *HWY_RESTRICT c, T *HWY_RESTRICT result) {
  const D d;
  const auto va = hn::Load(d, a);
  const auto vb = hn::Load(d, b);
  const auto vc = hn::Load(d, c);
  auto res = fma(d, va, vb, vc);
  hn::Store(res, d, result);
}

/* _<op>x<size>_<type> */

#define define_fp_xN_unary_op(type, name, size, op)                            \
  void _##op##x##size##_##name(const type *HWY_RESTRICT a,                     \
                               type *HWY_RESTRICT result) {                    \
    _##op##xN<type, size>(a, result);                                          \
  }

#define define_fp_xN_bin_op(type, name, size, op)                              \
  void _##op##x##size##_##name(const type *HWY_RESTRICT a,                     \
                               const type *HWY_RESTRICT b,                     \
                               type *HWY_RESTRICT result) {                    \
    _##op##xN<type, size>(a, b, result);                                       \
  }

#define define_fp_xN_ter_op(type, name, size, op)                              \
  void _##op##x##size##_##name(                                                \
      const type *HWY_RESTRICT a, const type *HWY_RESTRICT b,                  \
      const type *HWY_RESTRICT c, type *HWY_RESTRICT r) {                      \
    _##op##xN<type, size>(a, b, c, r);                                         \
  }

/* 32-bits */
#if HWY_MAX_BYTES >= 4
define_fp_xN_bin_op(float, f32, 1, add);
define_fp_xN_bin_op(float, f32, 1, sub);
define_fp_xN_bin_op(float, f32, 1, mul);
define_fp_xN_bin_op(float, f32, 1, div);
define_fp_xN_unary_op(float, f32, 1, sqrt);
define_fp_xN_ter_op(float, f32, 1, fma);
#endif

/* 64-bits */
#if HWY_MAX_BYTES >= 8
define_fp_xN_bin_op(double, f64, 1, add);
define_fp_xN_bin_op(double, f64, 1, sub);
define_fp_xN_bin_op(double, f64, 1, mul);
define_fp_xN_bin_op(double, f64, 1, div);
define_fp_xN_unary_op(double, f64, 1, sqrt);
define_fp_xN_ter_op(double, f64, 1, fma);

define_fp_xN_bin_op(float, f32, 2, add);
define_fp_xN_bin_op(float, f32, 2, sub);
define_fp_xN_bin_op(float, f32, 2, mul);
define_fp_xN_bin_op(float, f32, 2, div);
define_fp_xN_unary_op(float, f32, 2, sqrt);
define_fp_xN_ter_op(float, f32, 2, fma);
#endif

/* 128-bits */
#if HWY_MAX_BYTES >= 16
define_fp_xN_bin_op(double, f64, 2, add);
define_fp_xN_bin_op(double, f64, 2, sub);
define_fp_xN_bin_op(double, f64, 2, mul);
define_fp_xN_bin_op(double, f64, 2, div);
define_fp_xN_unary_op(double, f64, 2, sqrt);
define_fp_xN_ter_op(double, f64, 2, fma);

define_fp_xN_bin_op(float, f32, 4, add);
define_fp_xN_bin_op(float, f32, 4, sub);
define_fp_xN_bin_op(float, f32, 4, mul);
define_fp_xN_bin_op(float, f32, 4, div);
define_fp_xN_unary_op(float, f32, 4, sqrt);
define_fp_xN_ter_op(float, f32, 4, fma);
#endif

/* 256-bits */
#if HWY_MAX_BYTES >= 32
define_fp_xN_bin_op(double, f64, 4, add);
define_fp_xN_bin_op(double, f64, 4, sub);
define_fp_xN_bin_op(double, f64, 4, mul);
define_fp_xN_bin_op(double, f64, 4, div);
define_fp_xN_unary_op(double, f64, 4, sqrt);
define_fp_xN_ter_op(double, f64, 4, fma);

define_fp_xN_bin_op(float, f32, 8, add);
define_fp_xN_bin_op(float, f32, 8, sub);
define_fp_xN_bin_op(float, f32, 8, mul);
define_fp_xN_bin_op(float, f32, 8, div);
define_fp_xN_unary_op(float, f32, 8, sqrt);
define_fp_xN_ter_op(float, f32, 8, fma);
#endif

/* 512-bits */
#if HWY_MAX_BYTES >= 64
define_fp_xN_bin_op(double, f64, 8, add);
define_fp_xN_bin_op(double, f64, 8, sub);
define_fp_xN_bin_op(double, f64, 8, mul);
define_fp_xN_bin_op(double, f64, 8, div);
define_fp_xN_unary_op(double, f64, 8, sqrt);
define_fp_xN_ter_op(double, f64, 8, fma);

define_fp_xN_bin_op(float, f32, 16, add);
define_fp_xN_bin_op(float, f32, 16, sub);
define_fp_xN_bin_op(float, f32, 16, mul);
define_fp_xN_bin_op(float, f32, 16, div);
define_fp_xN_unary_op(float, f32, 16, sqrt);
define_fp_xN_ter_op(float, f32, 16, fma);
#endif


// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace prism::sr::vector::static_dispatch
HWY_AFTER_NAMESPACE();


#if HWY_ONCE

namespace prism::sr::vector::static_dispatch {

namespace {

/* 32-bits */
#if HWY_MAX_BYTES >= 4
HWY_EXPORT(_addx1_f32);
HWY_EXPORT(_subx1_f32);
HWY_EXPORT(_mulx1_f32);
HWY_EXPORT(_divx1_f32);
HWY_EXPORT(_sqrtx1_f32);
HWY_EXPORT(_fmax1_f32);
#endif

/* 64-bits */
#if HWY_MAX_BYTES >= 8
HWY_EXPORT(_addx1_f64);
HWY_EXPORT(_subx1_f64);
HWY_EXPORT(_mulx1_f64);
HWY_EXPORT(_divx1_f64);
HWY_EXPORT(_sqrtx1_f64);
HWY_EXPORT(_fmax1_f64);

HWY_EXPORT(_addx2_f32);
HWY_EXPORT(_subx2_f32);
HWY_EXPORT(_mulx2_f32);
HWY_EXPORT(_divx2_f32);
HWY_EXPORT(_sqrtx2_f32);
HWY_EXPORT(_fmax2_f32);
#endif

/* 128-bits */
#if HWY_MAX_BYTES >= 16
HWY_EXPORT(_addx2_f64);
HWY_EXPORT(_subx2_f64);
HWY_EXPORT(_mulx2_f64);
HWY_EXPORT(_divx2_f64);
HWY_EXPORT(_sqrtx2_f64);
HWY_EXPORT(_fmax2_f64);

HWY_EXPORT(_addx4_f32);
HWY_EXPORT(_subx4_f32);
HWY_EXPORT(_mulx4_f32);
HWY_EXPORT(_divx4_f32);
HWY_EXPORT(_sqrtx4_f32);
HWY_EXPORT(_fmax4_f32);
#endif

/* 256-bits */
#if (HWY_MAX_BYTES >= 32) && (HWY_TARGET != HWY_SSE4)
HWY_EXPORT(_addx4_f64);
HWY_EXPORT(_subx4_f64);
HWY_EXPORT(_mulx4_f64);
HWY_EXPORT(_divx4_f64);
HWY_EXPORT(_sqrtx4_f64);
HWY_EXPORT(_fmax4_f64);

HWY_EXPORT(_addx8_f32);
HWY_EXPORT(_subx8_f32);
HWY_EXPORT(_mulx8_f32);
HWY_EXPORT(_divx8_f32);
HWY_EXPORT(_sqrtx8_f32);
HWY_EXPORT(_fmax8_f32);
#endif

/* 512-bits */
#if HWY_MAX_BYTES >= 64
HWY_EXPORT(_addx8_f64);
HWY_EXPORT(_subx8_f64);
HWY_EXPORT(_mulx8_f64);
HWY_EXPORT(_divx8_f64);
HWY_EXPORT(_sqrtx8_f64);
HWY_EXPORT(_fmax8_f64);

HWY_EXPORT(_addx16_f32);
HWY_EXPORT(_subx16_f32);
HWY_EXPORT(_mulx16_f32);
HWY_EXPORT(_divx16_f32);
HWY_EXPORT(_sqrtx16_f32);
HWY_EXPORT(_fmax16_f32);
#endif

/* 1024-bits */
#if HWY_MAX_BYTES >= 128
HWY_EXPORT(_addx16_f64);
HWY_EXPORT(_subx16_f64);
HWY_EXPORT(_mulx16_f64);
HWY_EXPORT(_divx16_f64);
HWY_EXPORT(_sqrtx16_f64);
HWY_EXPORT(_fmax16_f64);
#endif

} // namespace

union f32x2_u {
  f32x2_v vector;
  alignas(8) float array[2];
};
union f64x2_u {
  f64x2_v vector;
  alignas(16) double array[2];
};
union f32x4_u {
  f32x4_v vector;
  alignas(16) float array[4];
};

union f64x4_u {
  f64x4_v vector;
  alignas(32) double array[4];
};
union f32x8_u {
  f32x8_v vector;
  alignas(32) float array[8];
};
union f64x8_u {
  f64x8_v vector;
  alignas(64) double array[8];
};
union f32x16_u {
  f32x16_v vector;
  alignas(64) float array[16];
};
union f64x16_u {
  f64x16_v vector;
  alignas(128) double array[16];
};

/* Single vector functions with static dispatch */

/* 64-bits */
#define define_static_unary_op(type, op, size)                            \
  type##x##size##_v op##type##x##size(const type##x##size##_v a) { \
    type##x##size##_u a_union = {.vector = a};                            \
    type##x##size##_u result_union;                                       \
    HWY_STATIC_DISPATCH(_##op##x##size##_##type)                          \
    (a_union.array, result_union.array);                                  \
    return result_union.vector;                                           \
  }

#define define_static_binary_op(type, op, size)                                 \
  type##x##size##_v op##type##x##size(const type##x##size##_v a,   \
                                             const type##x##size##_v b) { \
    type##x##size##_u a_union = {.vector = a};                            \
    type##x##size##_u b_union = {.vector = b};                            \
    type##x##size##_u result_union;                                       \
    HWY_STATIC_DISPATCH(_##op##x##size##_##type)                          \
    (a_union.array, b_union.array, result_union.array);                   \
    return result_union.vector;                                           \
  }

#define define_static_ternary_op(type, op, size)                                \
  type##x##size##_v op##type##x##size(const type##x##size##_v a,   \
                                             const type##x##size##_v b,   \
                                             const type##x##size##_v c) { \
    type##x##size##_u a_union = {.vector = a};                            \
    type##x##size##_u b_union = {.vector = b};                            \
    type##x##size##_u c_union = {.vector = c};                            \
    type##x##size##_u result_union;                                       \
    HWY_STATIC_DISPATCH(_##op##x##size##_##type)                          \
    (a_union.array, b_union.array, c_union.array, result_union.array);    \
    return result_union.vector;                                           \
  }


#if HWY_MAX_BYTES >= 8

/* binary32 */

define_static_binary_op(f32, add, 2);
define_static_binary_op(f32, sub, 2);
define_static_binary_op(f32, mul, 2);
define_static_binary_op(f32, div, 2);
define_static_unary_op(f32, sqrt, 2);
define_static_ternary_op(f32, fma, 2);

#endif

/* 128-bits */

#if HWY_MAX_BYTES >= 16

/* binary64 */

define_static_binary_op(f64, add, 2);
define_static_binary_op(f64, sub, 2);
define_static_binary_op(f64, mul, 2);
define_static_binary_op(f64, div, 2);
define_static_unary_op(f64, sqrt, 2);
define_static_ternary_op(f64, fma, 2);

/* binary32 */

define_static_binary_op(f32, add, 4);
define_static_binary_op(f32, sub, 4);
define_static_binary_op(f32, mul, 4);
define_static_binary_op(f32, div, 4);
define_static_unary_op(f32, sqrt, 4);
define_static_ternary_op(f32, fma, 4);

#endif

/* 256-bits */

#if HWY_MAX_BYTES >= 32

/* binary64 */

define_static_binary_op(f64, add, 4);
define_static_binary_op(f64, sub, 4);
define_static_binary_op(f64, mul, 4);
define_static_binary_op(f64, div, 4);
define_static_unary_op(f64, sqrt, 4);
define_static_ternary_op(f64, fma, 4);

/* binary32 */

define_static_binary_op(f32, add, 8);
define_static_binary_op(f32, sub, 8);
define_static_binary_op(f32, mul, 8);
define_static_binary_op(f32, div, 8);
define_static_unary_op(f32, sqrt, 8);
define_static_ternary_op(f32, fma, 8);

#endif

/* 512-bits */

#if HWY_MAX_BYTES >= 64

/* binary64 */

define_static_binary_op(f64, add, 8);
define_static_binary_op(f64, sub, 8);
define_static_binary_op(f64, mul, 8);
define_static_binary_op(f64, div, 8);
define_static_unary_op(f64, sqrt, 8);
define_static_ternary_op(f64, fma, 8);

/* binary32 */

define_static_binary_op(f32, add, 16);
define_static_binary_op(f32, sub, 16);
define_static_binary_op(f32, mul, 16);
define_static_binary_op(f32, div, 16);
define_static_unary_op(f32, sqrt, 16);
define_static_ternary_op(f32, fma, 16);

#endif // 512-bits

/* 1024-bits */
#if HWY_MAX_BYTES >= 128

/* binary64 */

define_static_binary_op(f64, add, 16);
define_static_binary_op(f64, sub, 16);
define_static_binary_op(f64, mul, 16);
define_static_binary_op(f64, div, 16);
define_static_unary_op(f64, sqrt, 16);
define_static_ternary_op(f64, fma, 16);

define_static_binary_op(f32, add, 32);
define_static_binary_op(f32, sub, 32);
define_static_binary_op(f32, mul, 32);
define_static_binary_op(f32, div, 32);
define_static_unary_op(f32, sqrt, 32);
define_static_ternary_op(f32, fma, 32);

#endif // 1024-bits

} // namespace prism::sr::vector::static_dispatch

#endif // HWY_ONCE