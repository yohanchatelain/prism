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
#include "src/xoshiro.h"
#include "src/random-inl.h"
#include "src/target_utils.h"
// clang-format on

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::ud::vector {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;

using RNG = hn::VectorXoshiro;

const auto seed = prism::get_user_seed();
thread_local RNG rng(seed, gettid() % getpid());

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V round(const V a) {
  debug_start();
  dbg::debug_vec<D>("[round] a", a);

  using DI = hn::RebindToSigned<D>;
  using DU = hn::RebindToUnsigned<D>;
  using U = hn::TFromD<DU>;
  const DI di{};
  const U u{};
  const D d{};

  auto is_not_zero = hn::Ne(a, hn::Set(d, 0));
  auto is_finite = hn::IsFinite(a);
  auto must_be_rounded = hn::And(is_finite, is_not_zero);

  // rand = 1 - 2 * (z & 1)
  auto z = rng(u);
  auto z_di = hn::ResizeBitCast(di, z);
  auto z_last_bit = hn::And(hn::Set(di, 0x1), z_di);
  auto z_last_bit_two = hn::ShiftLeft<1>(z_last_bit);
  auto rand = hn::Sub(hn::Set(di, 1), z_last_bit_two);

  auto a_di = hn::BitCast(di, a);
  auto a_rounded = hn::Add(a_di, rand);
  auto res = hn::BitCast(d, a_rounded);

  auto ret = hn::IfThenElse(must_be_rounded, res, a);

  dbg::debug_vec<D>("[round] res", ret);
  debug_end();

  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V add(const V a, const V b) {
  debug_start();
  dbg::debug_vec<D>("[add] a", a);
  dbg::debug_vec<D>("[add] b", b);

  V c = hn::Add(a, b);
  auto res = round<D>(c);

  dbg::debug_vec<D>("[add] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V sub(const V a, const V b) {
  debug_start();
  dbg::debug_vec<D>("[sub] a", a);
  dbg::debug_vec<D>("[sub] b", b);

  V c = hn::Sub(a, b);
  auto res = round<D>(c);

  dbg::debug_vec<D>("[sub] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V mul(const V a, const V b) {
  debug_start();
  dbg::debug_vec<D>("[mul] a", a);
  dbg::debug_vec<D>("[mul] b", b);

  V c = hn::Mul(a, b);
  auto res = round<D>(c);

  dbg::debug_vec<D>("[mul] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V div(const V a, const V b) {
  debug_start();
  dbg::debug_vec<D>("[div] a", a);
  dbg::debug_vec<D>("[div] b", b);

  V c = hn::Div(a, b);
  auto res = round<D>(c);

  dbg::debug_vec<D>("[div] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V sqrt(const V a) {
  debug_start();
  dbg::debug_vec<D>("[sqrt] a", a);

  V c = hn::Sqrt(a);
  auto res = round<D>(c);

  dbg::debug_vec<D>("[sqrt] res", res);
  debug_end();

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
V fma(const V a, const V b, const V c) {
  debug_start();
  dbg::debug_vec<D>("[fma] a", a);
  dbg::debug_vec<D>("[fma] b", b);
  dbg::debug_vec<D>("[fma] c", c);

  V d = hn::MulAdd(a, b, c);
  auto res = round<D>(d);

  dbg::debug_vec<D>("[fma] res", res);
  debug_end();

  return res;
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace prism::ud::vector
HWY_AFTER_NAMESPACE();

#endif // PRISM_UD_VECTOR_INL_H_
