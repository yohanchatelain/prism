#include <cmath>
#include <iostream>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#if defined(PRISM_SR_VECTOR_INL_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef PRISM_SR_VECTOR_INL_H_
#undef PRISM_SR_VECTOR_INL_H_
#else
#define PRISM_SR_VECTOR_INL_H_
#endif

// clang-format off
#include "hwy/highway.h"
#include "hwy/print-inl.h"
#include "src/utils.h"
#include "src/debug_vector-inl.h"
#include "src/xoshiro.h"
// clang-format on

HWY_BEFORE_NAMESPACE(); // at file scope
namespace prism::sr::vector::PRISM_DISPATCH {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;
namespace rng = prism::vector::xoshiro::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE void fasttwosum(const D d, const V a, const V b, V &sigma,
                             V &tau) {
  dbg::debug_msg("\n[twosum] START");
  dbg::debug_vec(d, "[twosum] a", a);
  dbg::debug_vec(d, "[twosum] b", b);

  const auto abs_a = hn::Abs(a);
  const auto abs_b = hn::Abs(b);
  const auto a_lt_b = hn::Lt(abs_a, abs_b);

  // Conditional swap if |a| < |b|
  const auto a_new = hn::IfThenElse(a_lt_b, b, a);
  const auto b_new = hn::IfThenElse(a_lt_b, a, b);

  sigma = hn::Add(a_new, b_new);
  const auto z = hn::Sub(sigma, a_new);
  tau = hn::Add(hn::Sub(a_new, hn::Sub(sigma, z)), hn::Sub(b_new, z));

  dbg::debug_vec(d, "[twosum] sigma", sigma);
  dbg::debug_vec(d, "[twosum] tau", tau);
  dbg::debug_msg("[twosum] END\n");
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
void twosum(const D d, const V a, const V b, V &sigma, V &tau) {
  dbg::debug_msg("\n[twosum] START");
  dbg::debug_vec(d, "[twosum] a", a);
  dbg::debug_vec(d, "[twosum] b", b);

  sigma = hn::Add(a, b);
  const auto a_p = hn::Sub(sigma, b);
  const auto b_p = hn::Sub(sigma, a_p);
  const auto d_a = hn::Sub(a, a_p);
  const auto d_b = hn::Sub(b, b_p);
  tau = hn::Add(d_a, d_b);

  dbg::debug_vec(d, "[twosum] sigma", sigma);
  dbg::debug_vec(d, "[twosum] tau", tau);
  dbg::debug_msg("[twosum] END\n");
}

inline int get_precision() {
  return std::is_same<float, float>::value ? 24 : 53;
}

/*
"Emulation of the FMA in rounded-to-nearest loating-point arithmetic"
Stef Graillat, Jean-Michel Muller
---
Algorithm 3 – Split(x, s). Veltkamp’s splitting algorithm. Returns a pair
(xh, xℓ) of FP numbers such that the significand of xh fits in s − p bits, the
significand of xℓ fits in s − 1 bits, and xh + xℓ = x.
Require: K = 2^s + 1
Require: 2 ≤ s ≤ p − 2
γ ← RN(K · x)
δ ← RN(x − γ)
xh ← RN(γ + δ)
xℓ ← RN(x − xh)
return (xh, xℓ)
*/
template <class D, class V = hn::TFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE void Split(const D d, const V x, V &xh, V &xl) {
  dbg::debug_msg("\n[Split] START");
  dbg::debug_vec(d, "[Split] x", x);

  const int s = (get_precision() + 1) / 2;
  const auto K = hn::Set(d, (1 << s) + 1);

  const auto gamma = hn::Mul(K, x);
  const auto delta = hn::Sub(x, gamma);
  xh = hn::Add(gamma, delta);
  xl = hn::Sub(x, xh);

  dbg::debug_vec(d, "[Split] xh", xh);
  dbg::debug_vec(d, "[Split] xl", xl);
  dbg::debug_msg("[Split] END\n");
}

/*
"Emulation of the FMA in rounded-to-nearest loating-point arithmetic"
Stef Graillat, Jean-Michel Muller
---
Algorithm 4 – DekkerProd(a, b). Dekker’s product. Returns a pair (πh, πℓ)
of FP numbers such that πh = RN(ab) and πh + πℓ = ab.
Require: s = ⌈p/2⌉
(ah, aℓ) ← Split(a, s)
(bh, bℓ) ← Split(b, s)
πh ← RN(a · b)
t1 ← RN(−πh + RN(ah · bh))
t2 ← RN(t1 + RN(ah · bℓ))
t3 ← RN(t2 + RN(aℓ · bh))
πℓ ← RN(t3 + RN(aℓ · bℓ))
return (πh, πℓ)
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE void DekkerProd(const D d, const V a, const V b, V &pi_h,
                             V &pi_l) {
  dbg::debug_msg("\n[DekkerProd] START");
  dbg::debug_vec(d, "[DekkerProd] a", a);
  dbg::debug_vec(d, "[DekkerProd] b", b);

  V ah, al;
  V bh, bl;
  Split(d, a, ah, al);
  Split(d, b, bh, bl);

  pi_h = hn::Mul(a, b);
  const auto neg_pi_h = hn::Neg(pi_h);
  const auto ah_bh = hn::Mul(ah, bh);
  const auto t1 = hn::Add(neg_pi_h, ah_bh);
  const auto ah_bl = hn::Mul(ah, bl);
  const auto t2 = hn::Add(t1, ah_bl);
  const auto al_bh = hn::Mul(al, bh);
  const auto t3 = hn::Add(t2, al_bh);
  const auto al_bl = hn::Mul(al, bl);
  pi_l = hn::Add(t3, al_bl);

  dbg::debug_vec(d, "[DekkerProd] pi_h", pi_h);
  dbg::debug_vec(d, "[DekkerProd] pi_l", pi_l);
  dbg::debug_msg("[DekkerProd] END\n");
}

/*
"Emulation of the FMA in rounded-to-nearest loating-point arithmetic"
Stef Graillat, Jean-Michel Muller
---
Algorithm 7 EmulFMA(a, b, c).
Require: P = 2^(p−1) + 1
Require: Q = 2^(p−1)
(πh, πℓ) ← DekkerProd(a, b)
(sh, sℓ) ← 2Sum(πh, c)
(vh, vℓ) ← 2Sum(πℓ, sℓ)
(zh, zℓ) ← Fast2Sum(sh, vh)
w ← RN(vℓ + zℓ)
L ← RN(P · w)
R ← RN(Q · w)
∆ ← RN(L − R)
d_temp_1 ← RN(zh + w)
if ∆ ≠ w then // mask
  return d_temp_1
else
  w' ← RN(3/2 · w)
  d_temp_2 ← RN(zh + w')
  if d_temp_2 = zh then // mask1
    return zh
  else
    δ ← RN(w − zℓ)
    t ← RN(vℓ − δ)
    if t = 0 then // mask2
      return d_temp_2
    else
      g ← RN(t · w)
      if g < 0 then // mask3
        return zh
      else
        return d_temp_2
      end if
    end if
  end if
end if
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V fma_emul(const D d, const V a, const V b, const V c) {
  dbg::debug_msg("\n[fma] START");
  dbg::debug_vec(d, "[fma] a", a);
  dbg::debug_vec(d, "[fma] b", b);
  dbg::debug_vec(d, "[fma] c", c);

  constexpr auto ulp = std::is_same<T, float>::value ? 0x1.0p-23f : 0x1.0p-52;
  const auto P = hn::Set(d, 1 + ulp);
  const auto Q = hn::Set(d, ulp);
  const auto q3_2 = hn::Set(d, 1.5);

  V pi_h, pi_l;
  V s_h, s_l;
  V v_h, v_l;
  V z_h, z_l;

  DekkerProd(d, a, b, pi_h, pi_l);
  twosum(d, pi_h, c, s_h, s_l);
  twosum(d, pi_l, s_l, v_h, v_l);
  twosum(d, s_h, v_h, z_h, z_l);

  const auto w = hn::Add(v_l, z_l);
  const auto L = hn::Mul(P, w);
  const auto R = hn::Mul(Q, w);
  const auto delta = hn::Sub(L, R);
  const auto d_temp_1 = hn::Add(z_h, w);
  const auto mask = hn::Ne(delta, w); // if delta != w then
  // else
  const auto w_prime = hn::Mul(q3_2, w);
  const auto d_temp_2 = hn::Add(z_h, w_prime);
  const auto mask1 = hn::Eq(d_temp_2, z_h); // if d_temp_2 = z_h then
  // else
  const auto delta_prime = hn::Sub(w, z_l);
  const auto t = hn::Sub(v_l, delta_prime);
  const auto zero_v = hn::Zero(d);
  const auto mask2 = hn::Eq(t, zero_v); // if t = 0 then
  // else
  const auto g = hn::Mul(t, w);
  const auto mask3 = hn::Lt(g, zero_v); // if g < 0 then

  const auto ret3 = hn::IfThenElse(mask3, z_h, d_temp_2);
  const auto ret2 = hn::IfThenElse(mask2, d_temp_2, ret3);
  const auto ret1 = hn::IfThenElse(mask1, z_h, ret2);
  const auto ret = hn::IfThenElse(mask, d_temp_1, ret1);

  const auto res = ret;

  dbg::debug_vec(d, "[fma] res", res);
  dbg::debug_msg("[fma] END\n");

  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE void twoprodfma(const D d, V a, V b, V &sigma, V &tau) {
  dbg::debug_msg("\n[twoprodfma] START");
  dbg::debug_vec(d, "[twoprodfma] a", a);
  dbg::debug_vec(d, "[twoprodfma] b", b);

  sigma = hn::Mul(a, b);
#if HWY_NATIVE_FMA
  tau = hn::MulSub(a, b, sigma); // Highway's MulSub is equivalent to a*b-c
#else
#if defined(HWY_COMPILE_ONLY_STATIC) or defined(WARN_FMA_EMULATION)
#warning "FMA not supported, using emulation (slow)"
#endif
  const auto hn = hn::Neg(sigma);
  tau = fma_emul(d, a, b, hn);
#endif

  dbg::debug_vec(d, "[twoprodfma] sigma", sigma);
  dbg::debug_vec(d, "[twoprodfma] tau", tau);
  dbg::debug_msg("[twoprodfma] END\n");
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V get_predecessor_abs(const D d, const V a) {
  constexpr T phi =
      std::is_same<T, float>::value ? 1.0f - 0x1.0p-24f : 1.0 - 0x1.0p-53;
  const auto phi_v = hn::Set(d, phi);
  const auto res = hn::Mul(a, phi_v);
  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename I = hn::TFromD<hn::RebindToSigned<D>>,
          typename VI = hn::Vec<hn::RebindToSigned<D>>>
HWY_NOINLINE VI get_exponent(const D d, const V a) {
  dbg::debug_msg("\n[get_exponent] START");

  using DU = hn::RebindToUnsigned<D>;
  using U = hn::TFromD<DU>;
  using DI = hn::RebindToSigned<D>;
  const DI di{};

  dbg::debug_vec(d, "[get_exponent] a", a);

  constexpr auto mantissa = hwy::MantissaBits<T>();
  constexpr auto exponent_mask = hwy::ExponentMask<T>();
  constexpr auto bias = static_cast<U>(hwy::MaxExponentField<T>()) >> 1;

  const auto zero_v = hn::Zero(di);
  const auto abs_a = hn::Abs(a);
  const auto bits = hn::BitCast(di, abs_a);
  const auto is_zero = hn::Eq(bits, zero_v);
  const auto exponent_mask_v = hn::Set(di, exponent_mask);
  const auto bits_exponent = hn::And(bits, exponent_mask_v);
  const auto mantissa_v = hn::Set(di, mantissa);
  const auto raw_exp = hn::Shr(bits_exponent, mantissa_v);
  const auto bias_v = hn::Set(di, bias);
  const auto exp = hn::Sub(raw_exp, bias_v);

  dbg::debug_mask(di, "[get_exponent] is_zero", is_zero);
  dbg::debug_vec(di, "[get_exponent] bits", bits);
  dbg::debug_vec(di, "[get_exponent] exponent_mask", exponent_mask_v);
  dbg::debug_vec(di, "[get_exponent] bits_exponent", bits_exponent);
  dbg::debug_vec(di, "[get_exponent] raw_exp", raw_exp);
  dbg::debug_vec(di, "[get_exponent] exp", exp, false);

  const auto res = hn::IfThenZeroElse(is_zero, exp);

  dbg::debug_vec(di, "[get_exponent] res", res, false);
  dbg::debug_msg("[get_exponent] END\n");

  return res;
}

// Computes 2^x, where x is an integer.
// Only works for x in the range of the exponent of the floating point type.
template <class D, class VI = hn::Vec<hn::Rebind<hwy::MakeSigned<D>, D>>,
          typename T = hn::TFromD<D>>
HWY_INLINE hn::Vec<D> FastPow2I(D d, VI x) {
  constexpr auto kOffsetS = std::is_same<T, float>::value ? 0x7F : 0x3FF;
  constexpr auto mantissa = std::is_same<T, float>::value ? 23 : 52;
  const hn::Rebind<hwy::MakeSigned<D>, D> di;
  const auto kOffset = Set(di, kOffsetS);
  const auto offset = Add(x, kOffset);
  const auto shift = hn::ShiftLeft<mantissa>(offset);
  return BitCast(d, shift);
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE hn::Vec<D> pow2(const D d, const V n) {
  dbg::debug_msg("\n[pow2] START");

  using DI = hn::RebindToSigned<D>;
  using I = hn::TFromD<DI>;
  const DI di{};

  dbg::debug_vec(di, "[pow2] n", n, false);

  constexpr I mantissa = prism::utils::IEEE754<T>::mantissa;
  constexpr I min_exponent = prism::utils::IEEE754<T>::min_exponent;

  // is_subnormal = n < min_exponent
  const auto min_exponent_v = hn::Set(di, min_exponent);
  const auto is_subnormal = hn::Lt(n, min_exponent_v);
  // precision_loss = is_subnormal ? min_exponent - n : 0

  const auto loss = hn::Sub(min_exponent_v, n);
  const auto precision_loss = hn::IfThenElseZero(is_subnormal, loss);

  // n_adjusted = is_subnormal ? 1 : n
  const auto one_v = hn::Set(di, 1);
  const auto n_adjusted = hn::IfThenElse(is_subnormal, one_v, n);

  dbg::debug_mask(di, "[pow2] is_subnormal", is_subnormal);

  const T one = 1.0;
  const auto one_as_int = reinterpret_cast<const I &>(one);
  const auto one_as_int_v = hn::Set(di, one_as_int);
  // res = is_subnormal ? 0 : 1
  const auto res = hn::IfThenZeroElse(is_subnormal, one_as_int_v);

  dbg::debug_vec(di, "[pow2] res", res);

  // shift = mantissa - precision_loss
  const auto mantissa_v = hn::Set(di, mantissa);
  const auto shift = hn::Sub(mantissa_v, precision_loss);

  dbg::debug_vec(di, "[pow2] n_adjusted", n_adjusted, false);
  dbg::debug_vec(di, "[pow2] precision_loss", precision_loss, false);
  dbg::debug_vec(di, "[pow2] shift", shift, false);

  // res = res + (n_adjusted << shift);
  const auto shift_adjusted = hn::Shl(n_adjusted, shift);
  const auto res_adjusted = hn::Add(res, shift_adjusted);
  const auto res_float = hn::BitCast(d, res_adjusted);

  dbg::debug_vec(di, "[pow2] n_adjusted << shift", shift_adjusted);
  dbg::debug_vec(di, "[pow2] res", res_adjusted);
  dbg::debug_vec(d, "[pow2] res", res_float);
  dbg::debug_msg("[pow2] END\n");

  return res_float;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V round(const D d, const V sigma, const V tau) {
  dbg::debug_msg("\n[sr_round] START");
  dbg::debug_vec(d, "[sr_round] sigma", sigma);
  dbg::debug_vec(d, "[sr_round] tau", tau);

  // get tag for int with same number of lanes as T
  using DI = hn::RebindToSigned<D>;
  const DI di{};

  constexpr int32_t mantissa = prism::utils::IEEE754<T>::mantissa;

  const auto zero = hn::Zero(d);
  const auto sign_tau = hn::Lt(tau, zero);
  const auto sign_sigma = hn::Lt(sigma, zero);

  const auto z_rng = rng::uniform(T{});
  const auto z = hn::ResizeBitCast(d, z_rng);

  const auto pred_sigma = get_predecessor_abs(d, sigma);

  // sign_diff = sign_tau != sign_sigma
  const auto sign_diff = hn::Xor(sign_tau, sign_sigma);

  const auto sign_diff_int = hn::RebindMask(di, sign_diff);

  const auto pred_sigma_exp = get_exponent(d, pred_sigma);
  const auto sigma_exp = get_exponent(d, sigma);
  const auto eta = hn::IfThenElse(sign_diff_int, pred_sigma_exp, sigma_exp);
  dbg::debug_vec(di, "[sr_round] eta", eta, false);

  const auto mantissa_v = hn::Set(di, mantissa);
  const auto exp = hn::Sub(eta, mantissa_v);
  const auto abs_ulp = pow2(d, exp);
  dbg::debug_vec(d, "[sr_round] |ulp|", abs_ulp);

  const auto ulp = hn::CopySign(abs_ulp, tau);
  dbg::debug_vec(d, "[sr_round] ulp", ulp);

  const auto pi = hn::Mul(ulp, z);
  dbg::debug_vec(d, "[sr_round] pi", pi);

  const auto tau_plus_pi = hn::Add(tau, pi);
  const auto abs_tau_plus_pi = hn::Abs(tau_plus_pi);
  dbg::debug_vec(d, "[sr_round] |tau|+pi", abs_tau_plus_pi);

  const auto ge = hn::Ge(abs_tau_plus_pi, abs_ulp);
  const auto round = hn::IfThenElse(ge, ulp, zero);
  dbg::debug_vec(d, "[sr_round] round", round);

  dbg::debug_msg("[sr_round] END\n");
  return round;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V add(const D d, const V a, const V b) {
  dbg::debug_msg("\n[sr_add] START");
  V sigma, tau;
  twosum(d, a, b, sigma, tau);
  const auto rounding = round(d, sigma, tau);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_add] res", ret);
  dbg::debug_msg("[sr_add] END\n");
  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V sub(const D d, const V a, const V b) {
  dbg::debug_msg("\n[sr_sub] START");
  const auto b_neg = hn::Neg(b);
  const auto ret = add(d, a, b_neg);
  dbg::debug_msg("[sr_sub] END\n");
  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V mul(const D d, const V a, const V b) {
  dbg::debug_msg("\n[sr_add] START");
  V sigma, tau;
  twoprodfma(d, a, b, sigma, tau);
  const auto rounding = round(d, sigma, tau);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_mul] res", ret);
  dbg::debug_msg("[sr_mul] END\n");

  return ret;
}

/*
Algorithm 6.9. Division With Stochastic Rounding Without
the Change of the Rounding Mode

1. Function Div2(a, b)
2. Compute σ = SR(a / b)
3. Z = rand()
4. σ = RN(a / b)
5. τ' = RN(-σ * b + a)
6. τ = RN(τ' / b)
7. round = SRround(σ, τ, Z)
8. σ = RN(σ + round)
9. return σ
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V div(const D d, const V a, const V b) {
  dbg::debug_msg("\n[sr_div] START");
  dbg::debug_vec(d, "[sr_div] a", a);
  dbg::debug_vec(d, "[sr_div] b", b);
  const auto sigma = hn::Div(a, b);
#if HWY_NATIVE_FMA
  const auto tau_p = hn::NegMulAdd(sigma, b, a);
#else
#if defined(HWY_COMPILE_ONLY_STATIC) or defined(WARN_FMA_EMULATION)
#warning "FMA not supported, using emulation (slow)"
#endif
  const auto neg_sigma = hn::Neg(sigma);
  const auto tau_p = fma_emul(d, neg_sigma, b, a);
#endif
  const auto tau = hn::Div(tau_p, b);
  const auto rounding = round(d, sigma, tau);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_div] σ", sigma);
  dbg::debug_vec(d, "[sr_div] τ'", tau_p);
  dbg::debug_vec(d, "[sr_div] τ", tau);
  dbg::debug_vec(d, "[sr_div] round", rounding);
  dbg::debug_vec(d, "[sr_div] res", ret);
  dbg::debug_msg("[sr_div] END\n");

  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V sqrt(const D d, const V a) {
  dbg::debug_msg("\n[sr_sqrt] START");

  const auto sigma = hn::Sqrt(a);
  // -sigma * sigma + a
  const auto tau_p = hn::NegMulAdd(sigma, sigma, a);
  const auto _div = hn::Div(tau_p, sigma);
  const auto half = hn::Set(d, 0.5);
  const auto tau = hn::Mul(half, _div);
  const auto rounding = round(d, sigma, tau);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_sqrt] res", ret);
  dbg::debug_msg("[sr_sqrt] END\n");

  return ret;
}

/*
"Exact and Approximated error of the FMA"
Sylvie Boldo, Jean-Michel Muller
---
Algorithm 5 (ErrFmaNearest):
  r1 = ◦(ax + y)
  (u1, u2) = Fast2Mult(a, x)
  (α1, α2) = 2Sum(y, u2)
  (β1, β2) = 2Sum(u1, α1)
  γ = ◦(◦(β1 − r1) + β2)
  r2 = ◦(γ + α2)
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_NOINLINE V fma(const D d, const V a, const V b, const V c) {
  dbg::debug_msg("\n[sr_fma] START");
  dbg::debug_vec(d, "[sr_fma] a", a);
  dbg::debug_vec(d, "[sr_fma] b", b);
  dbg::debug_vec(d, "[sr_fma] c", c);

#if HWY_NATIVE_FMA
  const auto r1 = hn::MulAdd(a, b, c);
#else
#if defined(HWY_COMPILE_ONLY_STATIC) or defined(WARN_FMA_EMULATION)
#warning "FMA not supported, using emulation (slow)"
#endif
  const auto r1 = fma_emul(d, a, b, c);
#endif
  V u1, u2, alpha1, alpha2, beta1, beta2, gamma, r2;
  twoprodfma(d, a, b, u1, u2);
  twosum(d, c, u2, alpha1, alpha2);
  twosum(d, u1, alpha1, beta1, beta2);
  const auto beta1_sub_r1 = hn::Sub(beta1, r1);
  gamma = hn::Add(beta1_sub_r1, beta2);
  r2 = hn::Add(gamma, alpha2);
  const auto rounding = round(d, r1, r2);
  const auto res = hn::Add(r1, rounding);
  dbg::debug_vec(d, "[sr_fma] res", res);
  dbg::debug_msg("[sr_fma] END\n");

  return res;
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace prism::sr::vector::PRISM_DISPATCH
HWY_AFTER_NAMESPACE();

#endif // PRISM_SR_VECTOR_INL_H_