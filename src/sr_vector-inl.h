#include <cmath>

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
namespace prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;
namespace rng = prism::vector::xoshiro::HWY_NAMESPACE;

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_INLINE void fasttwosum(const D d, const V a, const V b, V &sigma,
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

/*
Algorithm 5.1. TwoSum Augmented Addition
1. Function TwoSum(a, b)
2. Compute s, t such that s + t = a + b.
3. s = a + b
4. a' = s - b
5. b' = s - a'
6. δa = a - a'
7. δb = b - b'
8. t = δa + δb
9. return (s, t)
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_INLINE void twosum(const D d, const V a, const V b, V &sigma, V &tau) {
  dbg::debug_msg("\n[twosum] START");
  dbg::debug_vec(d, "[twosum] a", a);
  dbg::debug_vec(d, "[twosum] b", b);

  sigma = hn::Add(a, b);
  const auto a_p = hn::Sub(sigma, b);
  const auto b_p = hn::Sub(sigma, a_p);
  const auto d_a = hn::Sub(a, a_p);
  const auto d_b = hn::Sub(b, b_p);
  tau = hn::Add(d_a, d_b);
  tau = hn::IfThenElseZero(hn::IsFinite(sigma), tau);

  dbg::debug_vec(d, "[twosum] sigma", sigma);
  dbg::debug_vec(d, "[twosum] tau", tau);
  dbg::debug_msg("[twosum] END\n");
}

template <typename T> HWY_INLINE constexpr auto get_precision() -> int {
  return prism::utils::IEEE754<T>::precision;
}

/*

WARNING: Not correct if K.x overflows

"Emulation of the FMA in rounded-to-nearest floating-point arithmetic"
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
HWY_FLATTEN void Split(const D d, const V x, V &x_hi, V &x_lo) {
  dbg::debug_msg("\n[Split] START");
  dbg::debug_vec(d, "[Split] x", x);

  const uint32_t s = (get_precision<T>() + 1) / 2;
  const auto K = hn::Set(d, (1U << s) + 1);
  const auto s_str = "[Split] s " + std::to_string(s);
  dbg::debug_msg(s_str.c_str());
  dbg::debug_vec(d, "[Split] K", K);

  const auto gamma = hn::Mul(K, x);
  const auto delta = hn::Sub(x, gamma);
  dbg::debug_vec(d, "[Split] γ", gamma);
  dbg::debug_vec(d, "[Split] δ", delta);
  x_hi = hn::Add(gamma, delta);
  x_lo = hn::Sub(x, x_hi);

  dbg::debug_vec(d, "[Split] x_hi", x_hi);
  dbg::debug_vec(d, "[Split] x_lo", x_lo);
  dbg::debug_msg("[Split] END\n");
}

/**
"On various ways to split a floating-point number"
Claude-Pierre Jeannerod, Jean-Michel Muller, Paul Zimmermann‡
---
Algo from https://homepages.loria.fr/PZimmermann/papers/simul2.c

 same using bit manipulations
 double bitman(double x, double *xl) {
   union {
     double d;
     unsigned long n;
   } z;
   double xh;
   z.d = x;
   z.n &= ~0x7ffffffUL; / zero the low 27 bits /
   *xl = x - z.d;
   return z.d;
 }
**/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN void SplitBit(const D d, const V x, V &x_hi, V &x_lo) {
  dbg::debug_msg("\n[SplitBit] START");
  dbg::debug_vec(d, "[SplitBit] x", x);

  using DU = hn::RebindToUnsigned<D>;
  using U = hn::TFromD<DU>;
  const DU du{};

  constexpr U s = (get_precision<T>() + 1) / 2;
  const auto x_bits = hn::BitCast(du, x);
  const auto isfinite = hn::IsFinite(x);

  const auto one = static_cast<U>(1);
  const auto mask = hn::Set(du, ~((one << s) - 1));
  const auto xh_bits = hn::And(x_bits, mask);
  x_hi = hn::BitCast(d, xh_bits);
  x_hi = hn::IfThenElse(isfinite, x_hi, x);
  x_lo = hn::Sub(x, x_hi);
  x_lo = hn::IfThenElseZero(isfinite, x_lo);

  dbg::debug_vec(d, "[SplitBit] x_hi", x_hi);
  dbg::debug_vec(d, "[SplitBit] x_lo", x_lo);
  dbg::debug_msg("[SplitBit] END\n");
}

/*
"Emulation of the FMA in rounded-to-nearest floating-point arithmetic"
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
HWY_FLATTEN void DekkerProd(const D d, const V a, const V b, V &pi_hi,
                            V &pi_lo) {
  dbg::debug_msg("\n[DekkerProd] START");
  dbg::debug_vec(d, "[DekkerProd] a", a);
  dbg::debug_vec(d, "[DekkerProd] b", b);

  V ah;
  V al;
  V bh;
  V bl;
  SplitBit(d, a, ah, al);
  SplitBit(d, b, bh, bl);

  pi_hi = hn::Mul(a, b);
  const auto isfinite = hn::IsFinite(pi_hi);
  const auto t1 = hn::MulSub(ah, bh, pi_hi);
  const auto t2 = hn::MulAdd(ah, bl, t1);
  const auto t3 = hn::MulAdd(al, bh, t2);
  pi_lo = hn::MulAdd(al, bl, t3);
  pi_lo = hn::IfThenElseZero(isfinite, pi_lo);

  dbg::debug_vec(d, "[DekkerProd] pi_hi", pi_hi);
  dbg::debug_vec(d, "[DekkerProd] pi_lo", pi_lo);
  dbg::debug_msg("[DekkerProd] END\n");
}

/*
"Emulation of the FMA in rounded-to-nearest floating-point arithmetic"
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
HWY_FLATTEN auto fma_emul(const D d, const V a, const V b, const V c) -> V {
  dbg::debug_msg("\n[fma] START");
  dbg::debug_vec(d, "[fma] a", a);
  dbg::debug_vec(d, "[fma] b", b);
  dbg::debug_vec(d, "[fma] c", c);

  constexpr auto ulp = prism::utils::IEEE754<T>::ulp;
  const auto P = hn::Set(d, 1 + ulp);
  const auto Q = hn::Set(d, ulp);
  const auto q3_2 = hn::Set(d, 1.5);

  V pi_hi;
  V pi_lo;
  V s_hi;
  V s_lo;
  V v_hi;
  V v_lo;
  V z_hi;
  V z_lo;

  DekkerProd(d, a, b, pi_hi, pi_lo);
  twosum(d, pi_hi, c, s_hi, s_lo);
  twosum(d, pi_lo, s_lo, v_hi, v_lo);
  twosum(d, s_hi, v_hi, z_hi, z_lo);

  const auto w = hn::Add(v_lo, z_lo);
  const auto L = hn::Mul(P, w);
  const auto R = hn::Mul(Q, w);
  const auto delta = hn::Sub(L, R);
  const auto d_temp_1 = hn::Add(z_hi, w);
  const auto mask = hn::Ne(delta, w); // if delta != w then
  // else
  const auto w_prime = hn::Mul(q3_2, w);
  const auto d_temp_2 = hn::Add(z_hi, w_prime);
  const auto mask1 = hn::Eq(d_temp_2, z_hi); // if d_temp_2 = z_h then
  // else
  const auto delta_prime = hn::Sub(w, z_lo);
  const auto t = hn::Sub(v_lo, delta_prime);
  const auto zero_v = hn::Zero(d);
  const auto mask2 = hn::Eq(t, zero_v); // if t = 0 then
  // else
  const auto g = hn::Mul(t, w);
  const auto mask3 = hn::Lt(g, zero_v); // if g < 0 then

  const auto ret3 = hn::IfThenElse(mask3, z_hi, d_temp_2);
  const auto ret2 = hn::IfThenElse(mask2, d_temp_2, ret3);
  const auto ret1 = hn::IfThenElse(mask1, z_hi, ret2);
  const auto ret = hn::IfThenElse(mask, d_temp_1, ret1);

  const auto res = ret;

  dbg::debug_vec(d, "[fma] naive_fma", hn::MulAdd(a, b, c));
  dbg::debug_vec(d, "[fma] res", res);
  dbg::debug_msg("[fma] END\n");

  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN void twoprodfma(const D d, V a, V b, V &sigma, V &tau) {
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
HWY_FLATTEN auto get_predecessor_abs(const D d, const V a) -> V {
  constexpr auto halp_ulp =
      std::is_same<T, float>::value ? 0x1.0p-24F : 0x1.0p-53;
  constexpr T phi = 1.0 - halp_ulp;
  const auto phi_v = hn::Set(d, phi);
  const auto res = hn::Mul(a, phi_v);
  return res;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>,
          typename VI = hn::VFromD<hn::RebindToSigned<D>>>
HWY_INLINE auto get_exponent(const D d, const V a) -> VI {
  dbg::debug_msg("\n[get_exponent] START");

  using DI = hn::RebindToSigned<D>;
  const DI di{};

  dbg::debug_vec(d, "[get_exponent] a", a);

  constexpr auto mantissa = prism::utils::IEEE754<T>::mantissa;
  constexpr auto exponent_mask = prism::utils::IEEE754<T>::exponent_mask_scaled;
  constexpr auto bias = prism::utils::IEEE754<T>::bias;

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
HWY_FLATTEN auto FastPow2I(D d, VI x) -> hn::Vec<D> {
  constexpr auto kOffsetS = std::is_same<T, float>::value ? 0x7F : 0x3FF;
  constexpr auto mantissa = std::is_same<T, float>::value ? 23 : 52;
  const hn::Rebind<hwy::MakeSigned<D>, D> di;
  const auto kOffset = Set(di, kOffsetS);
  const auto offset = Add(x, kOffset);
  const auto shift = hn::ShiftLeft<mantissa>(offset);
  return BitCast(d, shift);
}

// Lookup table for common power-of-2 values
template <typename T>
struct Pow2LookupTable {
  static constexpr size_t kTableSize = 64;
  static constexpr T kMinExp = -32;
  static constexpr T kMaxExp = 31;
  
  alignas(64) static constexpr T table[kTableSize] = {
    // Pre-computed pow2 values for exponents -32 to 31
    2.328306436538696e-10, 4.656612873077393e-10, 9.313225746154785e-10, 1.862645149230957e-09,
    3.725290298461914e-09, 7.450580596923828e-09, 1.4901161193847656e-08, 2.9802322387695312e-08,
    5.9604644775390625e-08, 1.1920928955078125e-07, 2.384185791015625e-07, 4.76837158203125e-07,
    9.5367431640625e-07, 1.9073486328125e-06, 3.814697265625e-06, 7.62939453125e-06,
    1.52587890625e-05, 3.0517578125e-05, 6.103515625e-05, 0.0001220703125,
    0.000244140625, 0.00048828125, 0.0009765625, 0.001953125,
    0.00390625, 0.0078125, 0.015625, 0.03125,
    0.0625, 0.125, 0.25, 0.5,
    1.0, 2.0, 4.0, 8.0,
    16.0, 32.0, 64.0, 128.0,
    256.0, 512.0, 1024.0, 2048.0,
    4096.0, 8192.0, 16384.0, 32768.0,
    65536.0, 131072.0, 262144.0, 524288.0,
    1048576.0, 2097152.0, 4194304.0, 8388608.0,
    16777216.0, 33554432.0, 67108864.0, 134217728.0,
    268435456.0, 536870912.0, 1073741824.0, 2147483648.0
  };
};

template <class D, class VI = hn::VFromD<hn::RebindToSigned<D>>,
          typename T = hn::TFromD<D>>
HWY_INLINE auto pow2(const D d, const VI n) -> hn::VFromD<D> {
  dbg::debug_msg("\n[pow2] START");

  using DI = hn::DFromV<VI>;
  using I = hn::TFromV<VI>;
  const DI di{};

  dbg::debug_vec(di, "[pow2] n", n, false);

  constexpr I mantissa = prism::utils::IEEE754<T>::mantissa;
  constexpr I min_exponent = prism::utils::IEEE754<T>::min_exponent;
  
  // Fast path: check if we can use lookup table
  const auto min_table_exp = hn::Set(di, static_cast<I>(Pow2LookupTable<T>::kMinExp));
  const auto max_table_exp = hn::Set(di, static_cast<I>(Pow2LookupTable<T>::kMaxExp));
  const auto in_table_range = hn::And(hn::Ge(n, min_table_exp), hn::Le(n, max_table_exp));
  
  // If all values are in lookup table range, use fast path
  if (HWY_LIKELY(hn::AllTrue(di, in_table_range))) {
    // Use lookup table for common values
    const auto table_idx = hn::Add(n, hn::Set(di, -static_cast<I>(Pow2LookupTable<T>::kMinExp)));
    // For now, fall through to computation - vectorized table lookup would need gather operations
  }

  // is_subnormal = n < min_exponent
  const auto min_exponent_v = hn::Set(di, min_exponent);
  const auto is_subnormal = hn::Lt(n, min_exponent_v);
  // precision_loss = is_subnormal ? min_exponent - n : 0
  dbg::debug_mask(di, "[pow2] is_subnormal", is_subnormal);

  const auto loss = hn::Sub(min_exponent_v, n);
  const auto precision_loss = hn::IfThenElseZero(is_subnormal, loss);
  dbg::debug_vec(di, "[pow2] precision_loss", precision_loss, false);

  // n_adjusted = is_subnormal ? 1 : n
  const auto one_v = hn::Set(di, 1);
  const auto n_adjusted = hn::IfThenElse(is_subnormal, one_v, n);

  dbg::debug_vec(di, "[pow2] n_adjusted", n_adjusted, false);

  const utils::binaryN<T> one = {.f = 1.0};
  const auto one_as_int = one.i;
  const auto one_as_int_v = hn::Set(di, one_as_int);
  // res = is_subnormal ? 0 : 1
  const auto res = hn::IfThenZeroElse(is_subnormal, one_as_int_v);
  dbg::debug_vec(di, "[pow2] (fltasint) is_subnormal ? 0 : 1", res);

  // shift = mantissa - precision_loss
  const auto mantissa_v = hn::Set(di, mantissa);
  const auto shift = hn::Sub(mantissa_v, precision_loss);
  dbg::debug_vec(di, "[pow2] shift", shift, false);

  // res = res + (n_adjusted << shift);
  const auto shift_adjusted = hn::Shl(n_adjusted, shift);
  const auto res_adjusted = hn::Add(res, shift_adjusted);
  const auto res_float = hn::BitCast(d, res_adjusted);

  dbg::debug_vec(di, "[pow2] n_adjusted << shift", shift_adjusted);
  dbg::debug_vec(di, "[pow2] res + (n_adjusted << shift)", res_adjusted);
  dbg::debug_vec(d, "[pow2] (intasflt) res", res_float);

  dbg::debug_msg("[pow2] END\n");

  return res_float;
}

/*
Algorithm 6.6. A Helper Function for Stochastic Rounding
p = precision
ε = 2^(1−p)
1. Function SRround(σ, τ, Z)
2. Compute round.
3. if sign(τ) != sign(σ)
4.     η = get_exponent(pred(|σ|));
5. else
6.     η = get_exponent(σ);
7. ulp = sign(τ) * 2^η * ε;
8. π = ulp * Z;
9. if |RN(τ + π)| >= |ulp|
10.    round = ulp;
11. else
12.    round = 0;
13. return round;
*/
template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN auto round(const D d, const V sigma, const V tau) -> V {
  dbg::debug_msg("\n[sr_round] START");
  dbg::debug_vec(d, "[sr_round] σ", sigma);
  dbg::debug_vec(d, "[sr_round] τ", tau);

  // get tag for int with same number of lanes as T
  using DI = hn::RebindToSigned<D>;
  const DI di{};
  using I = hn::TFromD<DI>;

  constexpr I mantissa = prism::utils::IEEE754<T>::mantissa;

  const auto zero = hn::Zero(d);
  const auto sign_tau = hn::Lt(tau, zero);
  const auto sign_sigma = hn::Lt(sigma, zero);

  const auto z_rng = rng::uniform(T{});
  using DZ = hn::DFromV<decltype(z_rng)>;
  const auto z = hn::ResizeBitCast(d, z_rng);

  const auto pred_sigma = get_predecessor_abs(d, sigma);

  // sign_diff = sign_tau != sign_sigma
  const auto sign_diff = hn::Xor(sign_tau, sign_sigma);

  const auto sign_diff_int = hn::RebindMask(di, sign_diff);

  // Cache exponent calculations to avoid redundant computation
  thread_local static V last_sigma = hn::Zero(d);
  thread_local static VI last_sigma_exp = hn::Zero(hn::RebindToSigned<D>{}());
  thread_local static V last_pred_sigma = hn::Zero(d);
  thread_local static VI last_pred_sigma_exp = hn::Zero(hn::RebindToSigned<D>{}());
  
  VI sigma_exp, pred_sigma_exp;
  
  // Check if we can reuse cached sigma exponent
  if (HWY_LIKELY(hn::AllTrue(d, hn::Eq(sigma, last_sigma)))) {
    sigma_exp = last_sigma_exp;
  } else {
    sigma_exp = get_exponent(d, sigma);
    last_sigma = sigma;
    last_sigma_exp = sigma_exp;
  }
  
  // Check if we can reuse cached pred_sigma exponent
  if (HWY_LIKELY(hn::AllTrue(d, hn::Eq(pred_sigma, last_pred_sigma)))) {
    pred_sigma_exp = last_pred_sigma_exp;
  } else {
    pred_sigma_exp = get_exponent(d, pred_sigma);
    last_pred_sigma = pred_sigma;
    last_pred_sigma_exp = pred_sigma_exp;
  }
  
  const auto eta = hn::IfThenElse(sign_diff_int, pred_sigma_exp, sigma_exp);
  dbg::debug_vec(di, "[sr_round] η", eta, false);

  const auto mantissa_v = hn::Set(di, mantissa);
  const auto exp = hn::Sub(eta, mantissa_v);
  const auto abs_ulp = pow2(d, exp);
  dbg::debug_vec(d, "[sr_round] |ulp|", abs_ulp);

  const auto ulp = hn::CopySign(abs_ulp, tau);
  dbg::debug_vec(d, "[sr_round] ulp", ulp);

  const auto pi = hn::Mul(ulp, z);
  dbg::debug_vec(DZ{}, "[sr_round] z raw", z_rng);
  dbg::debug_vec(d, "[sr_round] z", z);
  dbg::debug_vec(d, "[sr_round] π", pi);

  const auto tau_plus_pi = hn::Add(tau, pi);
  const auto abs_tau_plus_pi = hn::Abs(tau_plus_pi);
  dbg::debug_vec(d, "[sr_round] |τ|+π", abs_tau_plus_pi);

  const auto ge = hn::Ge(abs_tau_plus_pi, abs_ulp);
  const auto round = hn::IfThenElse(ge, ulp, zero);
  dbg::debug_vec(d, "[sr_round] round", round);

  dbg::debug_msg("[sr_round] END\n");
  return round;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN auto add(const D d, const V a, const V b) -> V {
  dbg::debug_msg("\n[sr_add] START");
  V sigma;
  V tau;
  twosum(d, a, b, sigma, tau);
  const auto rounding = round(d, sigma, tau);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_add] res", ret);
  dbg::debug_msg("[sr_add] END\n");
  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN auto sub(const D d, const V a, const V b) -> V {
  dbg::debug_msg("\n[sr_sub] START");
  const auto b_neg = hn::Neg(b);
  const auto ret = add(d, a, b_neg);
  dbg::debug_msg("[sr_sub] END\n");
  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN auto mul(const D d, const V a, const V b) -> V {
  dbg::debug_msg("\n[sr_add] START");
  V sigma;
  V tau;
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
HWY_FLATTEN auto div(const D d, const V a, const V b) -> V {
  dbg::debug_msg("\n[sr_div] START");
  dbg::debug_vec(d, "[sr_div] a", a);
  dbg::debug_vec(d, "[sr_div] b", b);
  const auto sigma = hn::Div(a, b);
  dbg::debug_vec(d, "[sr_div] σ", sigma);
#if HWY_NATIVE_FMA
  const auto tau_p = hn::NegMulAdd(sigma, b, a);
#else
#if defined(HWY_COMPILE_ONLY_STATIC) or defined(WARN_FMA_EMULATION)
#warning "FMA not supported, using emulation (slow)"
#endif
  const auto neg_sigma = hn::Neg(sigma);
  const auto tau_p = fma_emul(d, neg_sigma, b, a);
#endif
  dbg::debug_vec(d, "[sr_div] τ'", tau_p);
  const auto tau = hn::Div(tau_p, b);
  dbg::debug_vec(d, "[sr_div] τ", tau);
  const auto rounding = round(d, sigma, tau);
  dbg::debug_vec(d, "[sr_div] round", rounding);
  const auto ret = hn::Add(sigma, rounding);
  dbg::debug_vec(d, "[sr_div] res", ret);
  dbg::debug_msg("[sr_div] END\n");

  return ret;
}

template <class D, class V = hn::VFromD<D>, typename T = hn::TFromD<D>>
HWY_FLATTEN auto sqrt(const D d, const V a) -> V {
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
HWY_FLATTEN auto fma(const D d, const V a, const V b, const V c) -> V {
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
  V u1;
  V u2;
  V alpha1;
  V alpha2;
  V beta1;
  V beta2;
  V gamma;
  V r2;
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
} // namespace prism::sr::vector::PRISM_DISPATCH::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#endif // PRISM_SR_VECTOR_INL_H_