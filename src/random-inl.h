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
#include <set>

#include "hwy/highway.h"

#include "hwy/aligned_allocator.h"

#include "src/debug_vector-inl.h"
#include "src/target_utils.h"

HWY_BEFORE_NAMESPACE(); // required if not using HWY_ATTR

namespace hwy {

namespace HWY_NAMESPACE { // required: unique per target

namespace hn = hwy::HWY_NAMESPACE;

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
constexpr float kMulConstF = 0.000000059604644775390625f;
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

  HWY_CXX14_CONSTEXPR std::uint64_t operator()() noexcept { return Next(); }

#if HWY_HAVE_FLOAT64
  HWY_CXX14_CONSTEXPR double Uniform() noexcept {
    return static_cast<double>(Next() >> 11) * kMulConst;
  }
#endif

  HWY_CXX14_CONSTEXPR std::array<std::uint64_t, 4> GetState() const {
    return {state_[0], state_[1], state_[2], state_[3]};
  }

  HWY_CXX17_CONSTEXPR void
  SetState(std::array<std::uint64_t, 4> state) noexcept {
    state_[0] = state[0];
    state_[1] = state[1];
    state_[2] = state[2];
    state_[3] = state[3];
  }

  static constexpr std::uint64_t StateSize() noexcept { return 4; }

  /* This is the jump function for the generator. It is equivalent to 2^128
   * calls to next(); it can be used to generate 2^128 non-overlapping
   * subsequences for parallel computations. */
  HWY_CXX14_CONSTEXPR void Jump() noexcept { Jump(kJump); }

  /* This is the long-jump function for the generator. It is equivalent to 2^192
   * calls to next(); it can be used to generate 2^64 starting points, from each
   * of which jump() will generate 2^64 non-overlapping subsequences for
   * parallel distributed computations. */
  HWY_CXX14_CONSTEXPR void LongJump() noexcept { Jump(kLongJump); }

  double UniformVec(const float) noexcept {
    union {
      std::uint64_t u64;
      std::uint32_t u32[2];
      double f64;
      float f32[2];

    } u;
    u.u64 = Next();
    u.f32[0] = (u.u32[0] >> 8) * kMulConstF;
    u.f32[1] = (u.u32[1] >> 8) * kMulConstF;
    return u.f64;
  }

  double UniformVec(const double) noexcept {
    return static_cast<double>(Next() >> 11) * kMulConst;
  }

private:
  std::uint64_t state_[4];

  static constexpr std::uint64_t Rotl(const std::uint64_t x, int k) noexcept {
    return (x << k) | (x >> (64 - k));
  }

  HWY_CXX14_CONSTEXPR std::uint64_t Next() noexcept {
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

    for (const std::uint64_t i : jumpArray)
      for (std::uint_fast8_t b = 0; b < 64; b++) {
        if (i & std::uint64_t{1UL} << b) {
          s0 ^= state_[0];
          s1 ^= state_[1];
          s2 ^= state_[2];
          s3 ^= state_[3];
        }
        Next();
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
  using VU64 = Vec<ScalableTag<std::uint64_t>>;
  using VU32 = Vec<ScalableTag<std::uint32_t>>;
  using StateType = AlignedNDArray<std::uint64_t, 2>;
  using VF32 = Vec<ScalableTag<float>>;
#if HWY_HAVE_FLOAT64
  using VF64 = Vec<ScalableTag<double>>;
#endif
public:
  explicit VectorXoshiro(const std::uint64_t seed,
                         const std::uint64_t threadNumber = 0)
      : state_{nullptr}, streams{0} {
#if PRISM_DEBUG
    std::cerr << "VectorXoshiro constructor called for target: " << target
              << std::endl;
#endif // PRISM_DEBUG
    // deferes initialization to avoid having illegal instructions called
    if (not isTargetSupported) {
#if PRISM_DEBUG
      std::cerr << "Target " << target << " not supported" << std::endl;
#endif // PRISM_DEBUG
      return;
    }

    initialize(seed, threadNumber);
  }

  void initialize(const std::uint64_t seed,
                  const std::uint64_t threadNumber = 0) {
    state_ = new StateType{
        {internal::Xoshiro::StateSize(), Lanes(ScalableTag<std::uint64_t>{})}};
    streams = state_->shape().back();

    internal::Xoshiro xoshiro{seed};

    for (std::uint64_t i = 0; i < threadNumber; ++i) {
      xoshiro.LongJump();
    }

    for (size_t i = 0UL; i < streams; ++i) {
      const auto state = xoshiro.GetState();
      for (size_t j = 0UL; j < internal::Xoshiro::StateSize(); ++j) {
        (*state_)[{j}][i] = state[j];
      }
      xoshiro.Jump();
    }
  }

  ~VectorXoshiro() {
#if PRISM_DEBUG
    std::cerr << "VectorXoshiro destructor called for target: " << target
              << std::endl;
#endif // PRISM_DEBUG
    if (not isTargetSupported) {
#if PRISM_DEBUG
      std::cerr << "Target " << target << " not supported" << std::endl;
#endif // PRISM_DEBUG
      return;
    }
    state_->~StateType();
  }

  HWY_INLINE VU32 operator()(std::uint32_t) noexcept {
    const auto result = Next();
    return BitCast(ScalableTag<std::uint32_t>{}, result);
  }

  HWY_INLINE VU64 operator()(std::uint64_t) noexcept { return Next(); }

  AlignedVector<std::uint32_t> operator()(std::uint32_t, const std::size_t n) {
    const auto u32_tag = ScalableTag<std::uint32_t>{};
    AlignedVector<std::uint32_t> result(2 * n);
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());
    for (std::uint64_t i = 0; i < n; i += Lanes(u32_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto next_u32 = BitCast(u32_tag, next);
      Store(next_u32, u32_tag, result.data() + i);
    }
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  AlignedVector<std::uint64_t> operator()(std::uint64_t, const std::size_t n) {
    AlignedVector<std::uint64_t> result(n);
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());
    for (std::uint64_t i = 0; i < n; i += Lanes(tag)) {
      const auto next = Update(s0, s1, s2, s3);
      Store(next, tag, result.data() + i);
    }
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  template <std::uint32_t N>
  std::array<std::uint32_t, 2 * N> operator()(std::uint32_t) noexcept {
    alignas(HWY_ALIGNMENT) std::array<std::uint32_t, 2 * N> result;
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<std::uint32_t> u32_tag{};
    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());
    for (std::uint64_t i = 0; i < N; i += Lanes(u32_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto next_u32 = BitCast(u32_tag, next);
      Store(next_u32, u32_tag, result.data() + i);
    }
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  template <std::uint64_t N>
  std::array<std::uint64_t, N> operator()(std::uint64_t) noexcept {
    alignas(HWY_ALIGNMENT) std::array<std::uint64_t, N> result;
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());
    for (std::uint64_t i = 0; i < N; i += Lanes(tag)) {
      const auto next = Update(s0, s1, s2, s3);
      Store(next, tag, result.data() + i);
    }
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  std::uint64_t StateSize() const noexcept {
    return streams * internal::Xoshiro::StateSize();
  }

  const StateType &GetState() const { return *state_; }

  template <typename T, class D = ScalableTag<T>, class V = Vec<D>>
  HWY_INLINE V Uniform(T) noexcept;
  template <typename T> AlignedVector<T> Uniform(T, const std::size_t n);
  template <typename T, std::uint64_t N> std::array<T, N> Uniform(T) noexcept;

  HWY_INLINE VF32 Uniform(float) noexcept {
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);
    const auto bits = Next();
    const auto bitscast = BitCast(u32_tag, bits);
    const auto bitsshift = ShiftRight<8>(bitscast);
    const auto real = ConvertTo(real_tag, bitsshift);
    return Mul(real, MUL_VALUE);
  }

  HWY_INLINE AlignedVector<float> Uniform(float, const std::size_t n) {
    AlignedVector<float> result(2 * n);
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);

    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());

    for (std::size_t i = 0; i < n; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = BitCast(u32_tag, next);
      const auto bitscast = ShiftRight<8>(bits);
      const auto real = ConvertTo(real_tag, bitscast);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  template <std::uint64_t N> std::array<float, 2 * N> Uniform(float) noexcept {
    alignas(HWY_ALIGNMENT) std::array<float, 2 * N> result;
    const ScalableTag<std::uint32_t> u32_tag{};
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<float> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConstF);

    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());

    for (std::uint32_t i = 0; i < N; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = BitCast(u32_tag, next);
      const auto bitscast = ShiftRight<8>(bits);
      const auto real = ConvertTo(real_tag, bitscast);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

#if HWY_HAVE_FLOAT64

  HWY_INLINE VF64 Uniform(double) noexcept {
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);
    const auto bits = ShiftRight<11>(Next());
    const auto real = ConvertTo(real_tag, bits);
    return Mul(real, MUL_VALUE);
  }

  AlignedVector<double> Uniform(double, const std::size_t n) {
    AlignedVector<double> result(n);
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);

    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());

    for (std::size_t i = 0; i < n; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = ShiftRight<11>(next);
      const auto real = ConvertTo(real_tag, bits);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

  template <std::uint64_t N> std::array<double, N> Uniform(double) noexcept {
    alignas(HWY_ALIGNMENT) std::array<double, N> result;
    const ScalableTag<std::uint64_t> tag{};
    const ScalableTag<double> real_tag{};
    const auto MUL_VALUE = Set(real_tag, internal::kMulConst);

    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());

    for (std::uint64_t i = 0; i < N; i += Lanes(real_tag)) {
      const auto next = Update(s0, s1, s2, s3);
      const auto bits = ShiftRight<11>(next);
      const auto real = ConvertTo(real_tag, bits);
      const auto uniform = Mul(real, MUL_VALUE);
      Store(uniform, real_tag, result.data() + i);
    }

    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }

#endif

private:
  bool isTargetSupported = prism::HWY_NAMESPACE::isCurrentTargetSupported();
  const std::string target = hwy::TargetName(HWY_TARGET);
  StateType *state_;
  std::uint64_t streams;

  HWY_INLINE static VU64 Update(VU64 &s0, VU64 &s1, VU64 &s2,
                                VU64 &s3) noexcept {
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

  HWY_INLINE VU64 Next() noexcept {
    const ScalableTag<std::uint64_t> tag{};
    auto s0 = Load(tag, (*state_)[{0}].data());
    auto s1 = Load(tag, (*state_)[{1}].data());
    auto s2 = Load(tag, (*state_)[{2}].data());
    auto s3 = Load(tag, (*state_)[{3}].data());
    auto result = Update(s0, s1, s2, s3);
    Store(s0, tag, (*state_)[{0}].data());
    Store(s1, tag, (*state_)[{1}].data());
    Store(s2, tag, (*state_)[{2}].data());
    Store(s3, tag, (*state_)[{3}].data());
    return result;
  }
};

template <std::uint64_t size = 1024> class CachedXoshiro {
public:
  using result_type = std::uint64_t;

  static constexpr result_type(min)() {
    return (std::numeric_limits<result_type>::min)();
  }

  static constexpr result_type(max)() {
    return (std::numeric_limits<result_type>::max)();
  }

  explicit CachedXoshiro(const result_type seed,
                         const result_type threadNumber = 0)
      : generator_{seed, threadNumber} {
#if PRISM_DEBUG
    std::cerr << "CachedXoshiro constructor called for target: " << target
              << std::endl;
#endif
    // deferes initialization to avoid having illegal instructions called
    if (not isTargetSupported) {
#if PRISM_DEBUG
      std::cerr << "Target " << target << " not supported" << std::endl;
#endif
      return;
    }

    cache_ = generator_.operator()<size>(result_type{});
    index_ = 0;
  }

  // delete operator
  ~CachedXoshiro() {
#if PRISM_DEBUG
    std::cerr << "CachedXoshiro destructor called for target: " << target
              << std::endl;
#endif
    if (not isTargetSupported) {
#if PRISM_DEBUG
      std::cerr << "Target " << target << " not supported" << std::endl;
#endif
      return;
    }
    generator_.~VectorXoshiro();
    cache_.~array();
  }

  result_type operator()() noexcept {
    if (HWY_UNLIKELY(index_ == size)) {
      cache_ = std::move(generator_.operator()<size>(result_type{}));
      index_ = 0;
    }
    return cache_[index_++];
  }

  double Uniform() noexcept {
    return static_cast<double>(operator()() >> 11) * internal::kMulConst;
  }

private:
  bool isTargetSupported = prism::HWY_NAMESPACE::isCurrentTargetSupported();
  const std::string target = hwy::TargetName(HWY_TARGET);
  VectorXoshiro generator_;
  alignas(HWY_ALIGNMENT) std::array<result_type, size> cache_;
  std::size_t index_;

  static_assert((size & (size - 1)) == 0 && size != 0,
                "only power of 2 are supported");
};

} // namespace HWY_NAMESPACE
} // namespace hwy

HWY_AFTER_NAMESPACE();

#endif // HIGHWAY_HWY_CONTRIB_MATH_MATH_INL_H_