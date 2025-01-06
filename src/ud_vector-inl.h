#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#if defined(PRISM_UD_VECTOR_INL_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_UD_VECTOR_INL_H_
#undef PRISM_UD_VECTOR_INL_H_
#else
#define PRISM_UD_VECTOR_INL_H_
#endif

// clang-format off
#include "hwy/highway.h"
#include "hwy/print-inl.h"
#include "src/debug_vector-inl.h"
#include "src/utils.h"
#include "src/target_utils.h"
#include "src/xoshiro.h"
// clang-format on

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism::ud::vector::PRISM_DISPATCH {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;
namespace rng = prism::vector::xoshiro::HWY_NAMESPACE;

template <class D, class V, typename T = hn::TFromD<D>>
V round(const D d, const V a) {
  debug_start();
  dbg::debug_vec(d, "[round] a", a);

  using DI = hn::RebindToSigned<D>;
  using DU = hn::RebindToUnsigned<D>;
  using U = hn::TFromD<DU>;
  const DI di{};
  const U u{};

  const auto is_not_zero = hn::Ne(a, hn::Set(d, 0));
  const auto is_finite = hn::IsFinite(a);
  const auto must_be_rounded = hn::And(is_finite, is_not_zero);

  // rand = 1 - 2 * (z & 1)
  const auto z = rng::random(u);
  const auto z_di = hn::ResizeBitCast(di, z);

  const auto one_di = hn::Set(di, 1);
  const auto z_last_bit = hn::And(one_di, z_di);
  const auto z_last_bit_two = hn::ShiftLeft<1>(z_last_bit);
  const auto rand = hn::Sub(one_di, z_last_bit_two);

  const auto a_di = hn::BitCast(di, a);
  const auto a_rounded = hn::Add(a_di, rand);
  const auto res = hn::BitCast(d, a_rounded);

  const auto ret = hn::IfThenElse(must_be_rounded, res, a);

  dbg::debug_vec(d, "[round] res", ret);
  debug_end();

  return ret;
}

template <class D, class V, typename T = hn::TFromD<D>>
V add(const D d, const V a, const V b) {
  debug_start();
  dbg::debug_vec(d, "[add] a", a);
  dbg::debug_vec(d, "[add] b", b);

  const V c = hn::Add(a, b);
  dbg::debug_vec(d, "[add] c", c);
  const auto res = round(d, c);

  dbg::debug_vec(d, "[add] res", res);
  debug_end();

  return res;
}

template <class D, class V, typename T = hn::TFromD<D>>
V sub(const D d, const V a, const V b) {
  debug_start();
  dbg::debug_vec(d, "[sub] a", a);
  dbg::debug_vec(d, "[sub] b", b);

  const V c = hn::Sub(a, b);
  dbg::debug_vec(d, "[sub] c", c);
  const auto res = round(d, c);

  dbg::debug_vec(d, "[sub] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V mul(const D d, const V a, const V b) {
  debug_start();
  dbg::debug_vec(d, "[mul] a", a);
  dbg::debug_vec(d, "[mul] b", b);

  const V c = hn::Mul(a, b);
  const auto res = round(d, c);

  dbg::debug_vec(d, "[mul] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V div(const D d, const V a, const V b) {
  debug_start();
  dbg::debug_vec(d, "[div] a", a);
  dbg::debug_vec(d, "[div] b", b);

  const V c = hn::Div(a, b);
  const auto res = round(d, c);

  dbg::debug_vec(d, "[div] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V sqrt(const D d, const V a) {
  debug_start();
  dbg::debug_vec(d, "[sqrt] a", a);

  const V c = hn::Sqrt(a);
  const auto res = round(d, c);

  dbg::debug_vec(d, "[sqrt] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V fma(const D d, const V a, const V b, const V c) {
  debug_start();
  dbg::debug_vec(d, "[fma] a", a);
  dbg::debug_vec(d, "[fma] b", b);
  dbg::debug_vec(d, "[fma] c", c);

  const V r = hn::MulAdd(a, b, c);
  const auto res = round(d, r);

  dbg::debug_vec(d, "[fma] res", res);
  debug_end();

  return res;
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace prism::ud::vector::PRISM_DISPATCH
HWY_AFTER_NAMESPACE();

#endif // PRISM_UD_VECTOR_INL_H_
