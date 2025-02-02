// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream> // cerr
#include <random>
#include <vector>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_xoshiro.cpp"  // NOLINT
#include "hwy/foreach_target.h"  // NOLINT IWYU pragma: keep
#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"
// clang-format on

#include "src/random-inl.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE();
namespace hwy {
namespace HWY_NAMESPACE { // required: unique per target
namespace {

namespace rng = prism::vector::xoshiro::HWY_NAMESPACE;

constexpr std::uint64_t tests = 1UL << 10;
constexpr std::uint64_t u64{};

typedef union {
  float f32[2];
  std::uint32_t u32[2];
  double f64;
} f32x2;

std::uint64_t GetSeed() { return static_cast<uint64_t>(std::time(nullptr)); }

void RngLoop(const std::uint64_t seed, std::uint64_t *HWY_RESTRICT result,
             const size_t size) {
  const ScalableTag<std::uint64_t> d;
  VectorXoshiro generator{seed};
  for (size_t i = 0; i < size; i += Lanes(d)) {
    Store(generator(u64), d, result + i);
  }
}

#if HWY_HAVE_FLOAT64
template <typename T>
void UniformLoop(const std::uint64_t seed, T *HWY_RESTRICT result,
                 const size_t size) {
  const ScalableTag<T> d;
  VectorXoshiro generator{seed};
  for (size_t i = 0; i < size; i += Lanes(d)) {
    Store(generator.Uniform(T{}), d, result + i);
  }
}
#endif

void TestSeeding() {
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  internal::Xoshiro reference{seed};
  const auto &state = generator.GetState();
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < lanes; ++i) {
    const auto &reference_state = reference.GetState();
    for (std::size_t j = 0UL; j < reference_state.size(); ++j) {
      if (state[{j}][i] != reference_state[j]) {
        std::cerr << "SEED: " << seed << "\n";
        std::cerr << "TEST SEEDING ERROR: ";
        std::cerr << "state[" << j << "][" << i << "] -> " << state[{j}][i]
                  << " != " << reference_state[j] << "\n";
        HWY_ASSERT(0);
      }
    }
    reference.Jump();
  }
}

void TestMultiThreadSeeding() {
  const std::uint64_t seed = GetSeed();
  const std::uint64_t threadId = std::random_device()() % 1000;
  VectorXoshiro generator{seed, threadId};
  internal::Xoshiro reference{seed};

  for (std::size_t i = 0UL; i < threadId; ++i) {
    reference.LongJump();
  }

  const auto &state = generator.GetState();
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < lanes; ++i) {
    const auto &reference_state = reference.GetState();
    for (std::size_t j = 0UL; j < reference_state.size(); ++j) {
      if (state[{j}][i] != reference_state[j]) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST SEEDING ERROR: ";
        std::cerr << "state[" << j << "][" << i << "] -> " << state[{j}][i]
                  << " != " << reference_state[j] << "\n";
        HWY_ASSERT(0);
      }
    }
    reference.Jump();
  }
}

void TestRandomUint64() {
  const std::uint64_t seed = GetSeed();
  const auto result_array = hwy::MakeUniqueAlignedArray<std::uint64_t>(tests);
  RngLoop(seed, result_array.get(), tests);
  std::vector<internal::Xoshiro> reference;
  reference.emplace_back(seed);
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 1UL; i < lanes; ++i) {
    auto rng = reference.back();
    rng.Jump();
    reference.emplace_back(rng);
  }

  for (std::size_t i = 0UL; i < tests; i += lanes) {
    for (std::size_t lane = 0UL; lane < lanes; ++lane) {
      const std::uint64_t result = reference[lane]();
      if (result_array[i + lane] != result) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UINT64 GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result
                  << std::endl;
        HWY_ASSERT(0);
      }
    }
  }
}

void TestUniformDist() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  const auto result_array = hwy::MakeUniqueAlignedArray<double>(tests);
  UniformLoop(seed, result_array.get(), tests);
  internal::Xoshiro reference{seed};
  const ScalableTag<double> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const double result = reference.Uniform();
    if (result_array[i] != result) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " != " << result << std::endl;
      HWY_ASSERT(0);
    }
    if (0.0 > result_array[i] or 1.0 <= result_array[i]) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " out of bounds" << std::endl;
      HWY_ASSERT(0);
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestUniformVecDist() {
#if HWY_HAVE_FLOAT64
  using D = ScalableTag<double>;
  const std::uint64_t seed = GetSeed();
  const auto result_array = hwy::MakeUniqueAlignedArray<double>(tests);
  UniformLoop(seed, result_array.get(), tests);
  internal::Xoshiro reference{seed};
  const D d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const double result = reference.UniformVec(double{});
    if (result_array[i] != result) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " != " << result << std::endl;
      HWY_ASSERT(0);
    }
    if (0.0 > result_array[i] or 1.0 <= result_array[i]) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " out of bounds" << std::endl;
      HWY_ASSERT(0);
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestUniformVecDistF32() {
#if HWY_HAVE_FLOAT64
  using D = ScalableTag<float>;
  const std::uint64_t seed = GetSeed();
  const auto result_array = hwy::MakeUniqueAlignedArray<float>(tests);
  UniformLoop(seed, result_array.get(), tests);
  internal::Xoshiro reference{seed};
  const D d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const f32x2 result = {.f64 = reference.UniformVec(float{})};
#if HWY_TARGET == HWY_SCALAR
    // Scalar implementation is not vectorized
    // so it returns 1 float per call
    const auto value_per_result = 1;
#else
    const auto value_per_result = 2;
#endif
    for (std::size_t lane = 0UL; lane < value_per_result; ++lane) {
      const float result_lane = result.f32[lane];
      if (result_array[i + lane] != result_lane) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result_lane
                  << std::endl;
        HWY_ASSERT(0);
      }
      if (0.0 > result_array[i + lane] or 1.0 <= result_array[i + lane]) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " out of bounds"
                  << std::endl;
        HWY_ASSERT(0);
      }
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestNextNRandomUint64() {
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.operator()(u64, tests);
  std::vector<internal::Xoshiro> reference;
  reference.emplace_back(seed);
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 1UL; i < lanes; ++i) {
    auto rng = reference.back();
    rng.Jump();
    reference.emplace_back(rng);
  }

  for (std::size_t i = 0UL; i < tests; i += lanes) {
    for (std::size_t lane = 0UL; lane < lanes; ++lane) {
      const std::uint64_t result = reference[lane]();
      if (result_array[i + lane] != result) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UINT64 GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result
                  << std::endl;
        HWY_ASSERT(0);
      }
    }
  }
}

void TestNextFixedNRandomUint64() {
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.operator()<tests>(u64);
  std::vector<internal::Xoshiro> reference;
  reference.emplace_back(seed);
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 1UL; i < lanes; ++i) {
    auto rng = reference.back();
    rng.Jump();
    reference.emplace_back(rng);
  }

  for (std::size_t i = 0UL; i < tests; i += lanes) {
    for (std::size_t lane = 0UL; lane < lanes; ++lane) {
      const std::uint64_t result = reference[lane]();
      if (result_array[i + lane] != result) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UINT64 GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result
                  << std::endl;

        HWY_ASSERT(0);
      }
    }
  }
}

void TestNextNUniformDist() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.Uniform(double{}, tests);
  internal::Xoshiro reference{seed};
  const ScalableTag<double> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const double result = reference.Uniform();
    if (result_array[i] != result) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " != " << result << std::endl;

      HWY_ASSERT(0);
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestNextNUniformVecDistF32() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.Uniform(float{}, tests);
  internal::Xoshiro reference{seed};
  const ScalableTag<float> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const f32x2 result = {.f64 = reference.UniformVec(float{})};
#if HWY_TARGET == HWY_SCALAR
    // Scalar implementation is not vectorized
    // so it returns 1 float per call
    const auto value_per_result = 1;
#else
    const auto value_per_result = 2;
#endif
    for (std::size_t lane = 0UL; lane < value_per_result; ++lane) {
      const float result_lane = result.f32[lane];
      if (result_array[i + lane] != result_lane) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result_lane
                  << std::endl;
        HWY_ASSERT(0);
      }
      if (0.0 > result_array[i + lane] or 1.0 <= result_array[i + lane]) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " out of bounds"
                  << std::endl;
        HWY_ASSERT(0);
      }
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestNextFixedNUniformDist() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.Uniform<tests>(double{});
  internal::Xoshiro reference{seed};
  const ScalableTag<double> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const double result = reference.Uniform();
    if (result_array[i] != result) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i << "] -> "
                << result_array[i] << " != " << result << std::endl;
      HWY_ASSERT(0);
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestNextFixedNUniformVecDistF32() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  VectorXoshiro generator{seed};
  const auto result_array = generator.Uniform<tests>(float{});
  internal::Xoshiro reference{seed};
  const ScalableTag<float> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 0UL; i < tests; i += lanes) {
    const f32x2 result = {.f64 = reference.UniformVec(float{})};
#if HWY_TARGET == HWY_SCALAR
    // Scalar implementation is not vectorized
    // so it returns 1 float per call
    const auto value_per_result = 1;
#else
    const auto value_per_result = 2;
#endif
    for (std::size_t lane = 0UL; lane < value_per_result; ++lane) {
      const float result_lane = result.f32[lane];
      if (result_array[i + lane] != result_lane) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " != " << result_lane
                  << std::endl;
        HWY_ASSERT(0);
      }
      if (0.0 > result_array[i + lane] or 1.0 <= result_array[i + lane]) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST UNIFORM GENERATOR ERROR: result_array[" << i + lane
                  << "] -> " << result_array[i + lane] << " out of bounds"
                  << std::endl;
        HWY_ASSERT(0);
      }
    }
  }
#endif // HWY_HAVE_FLOAT64
}

void TestCachedXorshiro() {
  const std::uint64_t seed = GetSeed();

  CachedXoshiro<> generator{seed};
  std::vector<internal::Xoshiro> reference;
  reference.emplace_back(seed);
  const ScalableTag<std::uint64_t> d;
  const std::size_t lanes = Lanes(d);
  for (std::size_t i = 1UL; i < lanes; ++i) {
    auto rng = reference.back();
    rng.Jump();
    reference.emplace_back(rng);
  }

  for (std::size_t i = 0UL; i < tests; i += lanes) {
    for (std::size_t lane = 0UL; lane < lanes; ++lane) {
      const std::uint64_t result = reference[lane]();
      const std::uint64_t got = generator();
      if (got != result) {
        std::cerr << "SEED: " << seed << std::endl;
        std::cerr << "TEST CachedXoshiro GENERATOR ERROR: result_array["
                  << i + lane << "] -> " << got << " != " << result
                  << std::endl;

        HWY_ASSERT(0);
      }
    }
  }
}

void TestUniformCachedXorshiro() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();

  CachedXoshiro<> generator{seed};
  std::uniform_real_distribution<double> distribution{0., 1.};
  for (std::size_t i = 0UL; i < tests; ++i) {
    const double result = distribution(generator);
    if (result < 0. || result >= 1.) {
      std::cerr << "SEED: " << seed << std::endl;
      std::cerr << "TEST CachedXoshiro GENERATOR ERROR: result_array[" << i
                << "] -> " << result << " not in interval [0, 1)" << std::endl;
      HWY_ASSERT(0);
    }
  }
#endif // HWY_HAVE_FLOAT64
}

} // namespace
// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace HWY_NAMESPACE
} // namespace hwy
HWY_AFTER_NAMESPACE(); // required if not using HWY_ATTR

#if HWY_ONCE
namespace hwy {
namespace {
// NOLINTBEGIN
HWY_BEFORE_TEST(HwyRandomTest);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestSeeding);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestMultiThreadSeeding);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestRandomUint64);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextNRandomUint64);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextFixedNRandomUint64);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestCachedXorshiro);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestUniformDist);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestUniformVecDist);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextNUniformDist);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextNUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextFixedNUniformDist);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestNextFixedNUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(HwyRandomTest, TestUniformCachedXorshiro);
HWY_AFTER_TEST();
// NOLINTEND
} // namespace
} // namespace hwy
HWY_TEST_MAIN();
#endif // HWY_ONCE
