/*
 * Original implementation written in 2019
 * by David Blackman and Sebastiano Vigna (vigna@acm.org)
 * Available at https://prng.di.unimi.it/ with creative commons license:
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 * See <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 * This implementation is a Vector port of the original implementation
 * written by Marco Barbone (m.barbone19@imperial.ac.uk).
 * I take no credit for the original implementation.
 * The code is provided as is and the original license applies.
 */

#if defined(HIGHWAY_HWY_CONTRIB_RANDOM_RANDOM_H_) ==                           \
    defined(HWY_TARGET_TOGGLE) // NOLINT
#ifdef HIGHWAY_HWY_CONTRIB_RANDOM_RANDOM_H_
#undef HIGHWAY_HWY_CONTRIB_RANDOM_RANDOM_H_
#else
#define HIGHWAY_HWY_CONTRIB_RANDOM_RANDOM_H_
#endif

#include <array>
#include <cstdint>
#include <limits>

#include "hwy/highway.h"

#include "hwy/aligned_allocator.h"

#include "src/debug_vector-inl.h"
#include "src/utils.h"

HWY_BEFORE_NAMESPACE(); // required if not using HWY_ATTR

namespace hwy {

namespace HWY_NAMESPACE { // required: unique per target

#ifdef PRISM_DEBUG_XOSHIRO
namespace dbg = prism::vector::HWY_NAMESPACE;
using namespace dbg;
#else
DISABLED_DEBUG_VEC
DISABLED_DEBUG_MSG
#endif

constexpr uint32_t expF32 = prism::utils::IEEE754<float>::exponent;
constexpr uint64_t expF64 = prism::utils::IEEE754<double>::exponent;

namespace internal {

namespace {
#if HWY_HAVE_FLOAT64
// C++ < 17 does not support hexfloat
#if __cpp_hex_float > 201603L
constexpr double kMulConst = 0x1.0p-53;
constexpr float kMulConstF = 0x1.0p-24f;
#else
constexpr double kMulConst =
    0.00000000000000011102230246251565404236316680908203125;
constexpr float kMulConstF = 0.000000059604644775390625F;
#endif // __cpp_hex_float

#endif // HWY_HAVE_FLOAT64

constexpr std::uint64_t kJump[] = {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
                                   0xa9582618e03fc9aa, 0x39abdc4529b1661c};

constexpr std::uint64_t kLongJump[] = {0x76e15d3efefdcbbf, 0xc5004e441c522fb3,
                                       0x77710069854ee241, 0x39109bb02acbe635};
} // namespace

class SplitMix64 {
public:
  constexpr explicit SplitMix64(const std::uint64_t state) noexcept
      : state_(state) {}

  HWY_CXX14_CONSTEXPR std::uint64_t operator()() {
    std::uint64_t z = (state_ += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
  }

private:
  std::uint64_t state_;
};

class Xoshiro {
public:
  HWY_CXX14_CONSTEXPR explicit Xoshiro(const std::uint64_t seed) noexcept
      : state_{} {
    SplitMix64 splitMix64{seed};
    for (auto &element : state_) {
      element = splitMix64();
    }
  }

  HWY_CXX14_CONSTEXPR explicit Xoshiro(const std::uint64_t seed,
                                       const std::uint64_t thread_id) noexcept
      : Xoshiro(seed) {
    for (auto i = UINT64_C(0); i < thread_id; ++i) {
      Jump();
    }
  }

  HWY_CXX14_CONSTEXPR auto operator()() noexcept -> std::uint64_t {
    return Next();
  }

#if HWY_HAVE_FLOAT64
  HWY_CXX14_CONSTEXPR auto Uniform() noexcept -> double {
    return static_cast<double>(Next() >> expF64) * kMulConst;
  }
#endif

  HWY_CXX14_CONSTEXPR auto GetState() const -> std::array<std::uint64_t, 4> {
    return {state_[0], state_[1], state_[2], state_[3]};
  }

  HWY_CXX17_CONSTEXPR void
  SetState(std::array<std::uint64_t, 4> state) noexcept {
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }

  static constexpr auto StateSize() noexcept -> std::uint64_t { return 4; }

  /* This is the jump function for the generator. It is equivalent to 2^128
   * calls to next(); it can be used to generate 2^128 non-overlapping
   * subsequences for parallel computations. */
  HWY_CXX14_CONSTEXPR void Jump() noexcept { Jump(kJump); }

  /* This is the long-jump function for the generator. It is equivalent to 2^192
   * calls to next(); it can be used to generate 2^64 starting points, from each
   * of which jump() will generate 2^64 non-overlapping subsequences for
   * parallel distributed computations. */
  HWY_CXX14_CONSTEXPR void LongJump() noexcept { Jump(kLongJump); }

  HWY_CXX14_CONSTEXPR auto
  UniformVec(const float /*unused*/) noexcept -> double {
    union {
      std::uint64_t u64;
      std::array<std::uint32_t, 2> u32;
      double f64;
      std::array<float, 2> f32;
    } u{Next()};
    u.f32[0] = (u.u32[0] >> expF32) * kMulConstF;
    u.f32[1] = (u.u32[1] >> expF32) * kMulConstF;
    return u.f64;
  }

  HWY_CXX14_CONSTEXPR auto UniformVec(const double) noexcept -> double {
    return Uniform();
  }

private:
  std::uint64_t state_[4];

  static constexpr auto Rotl(const std::uint64_t x,
                             int k) noexcept -> std::uint64_t {
    constexpr uint64_t sizeof_uint64_t = 64;
    return (x << k) | (x >> (sizeof_uint64_t - k));
  }

  HWY_CXX14_CONSTEXPR auto Next() noexcept -> std::uint64_t {
    const std::uint64_t result = Rotl(state_[0] + state_[3], 23) + state_[0];
    const std::uint64_t t = state_[1] << 17;

    state_[2] ^= state_[0];
    state_[3] ^= state_[1];
    state_[1] ^= state_[2];
    state_[0] ^= state_[3];

    state_[2] ^= t;

    state_[3] = Rotl(state_[3], 45);

    return result;
  }

  HWY_CXX14_CONSTEXPR void Jump(const std::uint64_t (&jumpArray)[4]) noexcept {
    std::uint64_t s0 = 0;
    std::uint64_t s1 = 0;
    std::uint64_t s2 = 0;
    std::uint64_t s3 = 0;

    for (const std::uint64_t i : jumpArray) {
      for (std::uint_fast8_t b = 0; b < 64; b++) {
        if (i & std::uint64_t{1UL} << b) {
          s0 ^= state_[0];
          s1 ^= state_[1];
          s2 ^= state_[2];
          s3 ^= state_[3];
        }
        Next();
      }
    }

    state_[0] = s0;
    state_[1] = s1;
    state_[2] = s2;
    state_[3] = s3;
  }
};

} // namespace internal

class VectorXoshiro {
private:
  using VU32 = Vec<ScalableTag<std::uint32_t>>;
  using VU64 = Vec<ScalableTag<std::uint64_t>>;
  using VF32 = Vec<ScalableTag<float>>;
  using StateType = AlignedNDArray<std::uint64_t, 2>;
#if HWY_HAVE_FLOAT64
  using VF64 = Vec<ScalableTag<double>>;

public:
#endif
  explicit VectorXoshiro(const std::uint64_t seed,
                         const std::uint64_t threadNumber = 0)
      : state_{{internal::Xoshiro::StateSize(),
                Lanes(ScalableTag<std::uint64_t>{})}},
        streams{state_.shape().back()} {
    internal::Xoshiro xoshiro{seed};

    for (std::uint64_t i = 0; i < threadNumber; ++i) {
      xoshiro.LongJump();
    }

    for (size_t i = 0UL; i < streams; ++i) {
      const auto state = xoshiro.GetState();
      for (size_t j = 0UL; j < internal::Xoshiro::StateSize(); ++j) {
        state_[{j}][i] = state[j];
      }
      xoshiro.Jump();
    }
#if PRISM_RNG_DEBUG
    fprintf(stderr,
            "[PRISM VectorXoshiro] VectorXoshiro initialized at %p: %lu "
            "streams, %lu states, %lu state size\n",
            this, streams, state_.size(), StateSize());
#endif
  }

  HWY_INLINE auto operator()(std::uint32_t /*unused*/) noexcept -> VU32 {
    const auto result = Next();
    return BitCast(ScalableTag<std::uint32_t>{}, result);
  }

  HWY_INLINE auto operator()(std::uint64_t /*unused*/) noexcept -> VU64 {
    return Next();
  }

  auto operator()(std::uint32_t /*unused*/,
                  const std::size_t n) -> AlignedVector<std::uint32_t> {
    debug_msg("\n[VectorXoshiro] START AlignedVector<uint32_t>");
    const auto u32_tag = ScalableTag<std::uint32_t>{};
    AlignedVector<std::uint32_t> result(2 * n);
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    for (std::uint64_t i = 0; i < n; i += Lanes(u32_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto next_u32 = BitCast(u32_tag, next);
      debug_vec(tag, "[VectorXoshiro] next", next);
      debug_vec(u32_tag, "[VectorXoshiro] next_u32", next_u32);
      Store(next_u32, u32_tag, result.data() + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    debug_msg("[VectorXoshiro] END AlignedVector<uint32_t>");
    return result;
  }

  auto operator()(std::uint64_t /*unused*/,
                  const std::size_t n) -> AlignedVector<std::uint64_t> {
    AlignedVector<std::uint64_t> result(n);
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    for (std::uint64_t i = 0; i < n; i += Lanes(tag)) {
      const auto next = Update(s0, s1, s2, s3);
      Store(next, tag, result.data() + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }

  template <std::uint32_t N>
  auto operator()(std::uint32_t /*unused*/) noexcept
      -> std::array<std::uint32_t, 2 * N> {
    debug_msg("\n[VectorXoshiro] START array<uint32_t>");
    alignas(HWY_ALIGNMENT) std::array<std::uint32_t, 2 * N> result;
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<std::uint32_t> u32_tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    for (std::uint64_t i = 0; i < N; i += Lanes(u32_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto next_u32 = BitCast(u32_tag, next);
      debug_vec(tag, "[VectorXoshiro] next", next);
      debug_vec(u32_tag, "[VectorXoshiro] next_u32", next_u32);
      Store(next_u32, u32_tag, result.data() + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    debug_msg("[VectorXoshiro] END array<uint32_t>");
    return result;
  }

  template <std::uint64_t N>
  auto operator()(std::uint64_t /*unused*/) noexcept
      -> std::array<std::uint64_t, N> {
    alignas(HWY_ALIGNMENT) std::array<std::uint64_t, N> result;
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    for (std::uint64_t i = 0; i < N; i += Lanes(tag)) {
      const auto next = Update(s0, s1, s2, s3);
      Store(next, tag, result.data() + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }

  template <std::uint64_t N> void fill(std::uint64_t *HWY_RESTRICT data) {
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    for (std::uint64_t i = 0; i < N; i += Lanes(tag)) {
      const auto next = Update(s0, s1, s2, s3);
      Store(next, tag, data + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
  }

  [[nodiscard]] auto StateSize() const noexcept -> std::uint64_t {
    return streams * internal::Xoshiro::StateSize();
  }

  [[nodiscard]] auto GetState() const -> const StateType & { return state_; }

  HWY_INLINE auto Uniform(float) noexcept -> VF32 {
    debug_msg("\n[VectorXoshiro] START Uniform<float>");
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);
    const auto bits = Next();
    const auto bitscast = BitCast(u32_tag, bits);
    debug_vec(tag, "[VectorXoshiro] bits", bits);
    debug_vec(u32_tag, "[VectorXoshiro] u32 bits", bitscast);
    const auto bitsshift = ShiftRight<expF32>(bitscast);
    const auto real = ConvertTo(real_tag, bitsshift);
    debug_vec(real_tag, "[VectorXoshiro] real", real);
    const auto res = Mul(real, MUL_VALUE);
    debug_vec(real_tag, "[VectorXoshiro] res", res);
    return res;
  }

  HWY_INLINE auto Uniform(float /*unused*/,
                          const std::size_t n) -> AlignedVector<float> {
    debug_msg("\n[VectorXoshiro] START AlignedVector<float>");
    AlignedVector<float> result(2 * n);
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);

    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());

    for (std::size_t i = 0; i < n; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = BitCast(u32_tag, next);
      const auto bitscast = ShiftRight<expF32>(bits);
      const auto real = ConvertTo(real_tag, bitscast);
      const auto uniform = Mul(real, MUL_VALUE);
      debug_vec(tag, "[VectorXoshiro] bits", next);
      debug_vec(u32_tag, "[VectorXoshiro] bits u32", bitscast);
      debug_vec(real_tag, "[VectorXoshiro] real", real);
      debug_vec(real_tag, "[VectorXoshiro] uniform", uniform);
      Store(uniform, real_tag, result.data() + i);
    }
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    debug_msg("[VectorXoshiro] END AlignedVector<float>");
    return result;
  }

  template <std::uint64_t N>
  auto Uniform(float /*unused*/) noexcept -> std::array<float, 2 * N> {
    alignas(HWY_ALIGNMENT) std::array<float, 2 * N> result;
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);

    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());

    for (std::uint32_t i = 0; i < N; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = BitCast(u32_tag, next);
      const auto bitscast = ShiftRight<expF32>(bits);
      const auto real = ConvertTo(real_tag, bitscast);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }

#if HWY_HAVE_FLOAT64

  auto Uniform(double /*unused*/) noexcept -> VF64 {
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);
    const auto bits = Next();
    const auto bits_s = ShiftRight<expF64>(bits);
    const auto real = ConvertTo(real_tag, bits_s);
    return Mul(real, MUL_VALUE);
  }

  auto Uniform(double /*unused*/,
               const std::size_t n) -> AlignedVector<double> {
    AlignedVector<double> result(n);
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);

    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());

    for (std::size_t i = 0; i < n; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = ShiftRight<expF64>(next);
      const auto real = ConvertTo(real_tag, bits);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }

  template <std::uint64_t N>
  auto Uniform(double /*unused*/) noexcept -> std::array<double, N> {
    alignas(HWY_ALIGNMENT) std::array<double, N> result;
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);

    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());

    for (std::uint64_t i = 0; i < N; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = ShiftRight<expF64>(next);
      const auto real = ConvertTo(real_tag, bits);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }

#endif

private:
  StateType state_;
  std::uint64_t streams;

  HWY_INLINE static auto Update(VU64 &s0, VU64 &s1, VU64 &s2,
                                VU64 &s3) noexcept -> VU64 {
    const auto result = Add(RotateRight<41>(Add(s0, s3)), s0);
    const auto t = ShiftLeft<17>(s1);
    s2 = Xor(s2, s0);
    s3 = Xor(s3, s1);
    s1 = Xor(s1, s2);
    s0 = Xor(s0, s3);
    s2 = Xor(s2, t);
    s3 = RotateRight<19>(s3);
    return result;
  }

  HWY_INLINE auto Next() noexcept -> VU64 {
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, state_[{0}].data());
    auto s1 = Load(tag, state_[{1}].data());
    auto s2 = Load(tag, state_[{2}].data());
    auto s3 = Load(tag, state_[{3}].data());
    auto result = Update(s0, s1, s2, s3);
    Store(s0, tag, state_[{0}].data());
    Store(s1, tag, state_[{1}].data());
    Store(s2, tag, state_[{2}].data());
    Store(s3, tag, state_[{3}].data());
    return result;
  }
};

constexpr auto kCachedXoshiroSize = 1024;

template <std::uint64_t size = kCachedXoshiroSize> class CachedXoshiro {
public:
  using result_type = std::uint64_t;

  static constexpr auto(min)() -> result_type {
    return (std::numeric_limits<result_type>::min)();
  }

  static constexpr auto(max)() -> result_type {
    return (std::numeric_limits<result_type>::max)();
  }

  explicit CachedXoshiro(const result_type seed,
                         const result_type threadNumber = 0)
      : generator_{seed, threadNumber},
        cache_{generator_.operator()<size>(result_type{})} {

#if PRISM_RNG_DEBUG
    fprintf(stderr,
            "[PRISM CachedXoshiro] CachedXoshiro initialized at %p: %lu "
            "streams, %lu states\n",
            this, size, generator_.StateSize());
#endif
  }

  auto operator()() noexcept -> result_type {
    if (HWY_UNLIKELY(index_ == size)) {
      // cache_ = std::move(generator_.operator()<size>(result_type{}));
      generator_.fill<size>(cache_.data());
      index_ = 0;
#if PRISM_RNG_DEBUG
      static int call_count = 0;
      fprintf(stderr,
              "[PRISM CachedXoshiro] [%d] CachedXoshiro cache exhausted, "
              "generating new cache at %p\n",
              ++call_count, cache_.data());
#endif
    }
    return cache_[index_++];
  }

  auto Uniform() noexcept -> double {
    return static_cast<double>(operator()() >> expF64) * internal::kMulConst;
  }

private:
  VectorXoshiro generator_;
  alignas(HWY_ALIGNMENT) std::array<result_type, size> cache_;
  std::size_t index_{};

  static_assert((size & (size - 1)) == 0 && size != 0,
                "only power of 2 are supported");
};

} // namespace HWY_NAMESPACE
} // namespace hwy

HWY_AFTER_NAMESPACE();

#endif // HIGHWAY_HWY_CONTRIB_MATH_MATH_INL_H_