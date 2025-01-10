// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream> // cerr
#include <random>
#include <vector>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_xoshiro_api.cpp"  // NOLINT
#include "hwy/foreach_target.h"  // IWYU pragma: keep 
#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"
// clang-format on

#include "src/debug_vector-inl.h"
#include "src/random-inl.h"
#include "src/xoshiro.h"

HWY_BEFORE_NAMESPACE();
namespace hwy::HWY_NAMESPACE { // required: unique per target
namespace {

namespace rng_vector = prism::vector::xoshiro::HWY_NAMESPACE;
namespace rng_scalar = prism::scalar::xoshiro::HWY_NAMESPACE;
namespace dbg = prism::vector::HWY_NAMESPACE;

constexpr std::uint64_t tests = 1UL << 10;
constexpr std::uint64_t u64{};

using f32x2 = union {
  double f64;
  std::uint64_t u64;
  std::array<float, 2> f32;
  std::array<std::uint32_t, 2> u32;
};

auto GetSeed() -> std::uint64_t {
  return static_cast<uint64_t>(std::time(nullptr));
}

void InitRngVector(const std::uint64_t seed, const std::uint64_t tid = 0) {
  rng_vector::internal::init_rng(seed, tid);
}

template <typename T>
void RngLoopVector(const std::uint64_t seed, T *HWY_RESTRICT result,
                   const size_t size) {
  const ScalableTag<T> d;
  InitRngVector(seed);
  for (size_t i = 0; i < size; i += Lanes(d)) {
    const auto z = rng_vector::random(T{});
    Store(z, d, result + i);
  }
}

#if HWY_HAVE_FLOAT64
template <typename T>
void UniformLoopVector(const std::uint64_t seed, T *HWY_RESTRICT result,
                       const size_t size) {
  const ScalableTag<T> d;
  InitRngVector(seed);
  for (size_t i = 0; i < size; i += Lanes(d)) {
    const auto z = rng_vector::uniform(T{});
    Store(z, d, result + i);
  }
}
#endif

void TestSeedingAPI() {
  const std::uint64_t seed = GetSeed();
  InitRngVector(seed);
  internal::Xoshiro reference{seed};
  const auto &state = rng_vector::internal::get_rng()->GetState();
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
  InitRngVector(seed, threadId);
  internal::Xoshiro reference{seed};

  for (std::size_t i = 0UL; i < threadId; ++i) {
    reference.LongJump();
  }

  const auto &state = rng_vector::internal::get_rng()->GetState();
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
  RngLoopVector<std::uint64_t>(seed, result_array.get(), tests);
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

void TestRandomUint32() {
  constexpr auto tests_u32 = 2 * tests;
  const std::uint64_t seed = GetSeed();
  const auto result_array =
      hwy::MakeUniqueAlignedArray<std::uint32_t>(tests_u32);
  RngLoopVector<std::uint32_t>(seed, result_array.get(), tests_u32);

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
      const f32x2 result = {.u64 = reference[lane]()};
#if HWY_TARGET == HWY_SCALAR
      // Scalar implementation is not vectorized
      // so it returns 1 value per call
      const auto value_per_result = 1;
#else
      const auto value_per_result = 2;
#endif
      for (std::size_t k = 0UL; k < value_per_result; ++k) {
        const std::uint32_t result_lane = result.u32[k];
        const auto result_idx =
            i * value_per_result + value_per_result * lane + k;
        if (result_array[result_idx] != result_lane) {
          std::cerr << "SEED: " << seed << std::endl;
          std::cerr << "TEST UINT64 GENERATOR ERROR: result_array["
                    << result_idx << "] -> " << result_array[result_idx]
                    << " != " << result_lane << std::endl;
          HWY_ASSERT(0);
        }
      }
    }
  }
}

void TestUniformDist() {
#if HWY_HAVE_FLOAT64
  const std::uint64_t seed = GetSeed();
  const auto result_array = hwy::MakeUniqueAlignedArray<double>(tests);
  UniformLoopVector(seed, result_array.get(), tests);
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
  UniformLoopVector(seed, result_array.get(), tests);
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
  UniformLoopVector(seed, result_array.get(), tests);
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->operator()(u64, tests);
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->operator()<tests>(u64);
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->Uniform(double{}, tests);
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->Uniform(float{}, tests);
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->Uniform<tests>(double{});
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
  rng_vector::internal::init_rng(seed, 0);
  auto generator = rng_vector::internal::get_rng();
  const auto result_array = generator->Uniform<tests>(float{});
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
  rng_scalar::internal::init_rng(seed, 0);
  auto generator = rng_scalar::internal::get_rng();
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
      const std::uint64_t got = generator->operator()();
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

  rng_scalar::internal::init_rng(seed, 0);
  auto generator = rng_scalar::internal::get_rng();
  std::uniform_real_distribution<double> distribution{0., 1.};
  for (std::size_t i = 0UL; i < tests; ++i) {
    const double result = distribution(*generator);
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
} // namespace hwy::HWY_NAMESPACE
HWY_AFTER_NAMESPACE(); // required if not using HWY_ATTR

#if HWY_ONCE
namespace hwy::HWY_NAMESPACE {
namespace {
HWY_BEFORE_TEST(PRISMXoshiroAPITest);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestSeedingAPI);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestMultiThreadSeeding);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestRandomUint64);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestRandomUint32);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestUniformDist);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestUniformVecDist);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextNRandomUint64);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextFixedNRandomUint64);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextNUniformDist);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextNUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextFixedNUniformDist);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestNextFixedNUniformVecDistF32);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestCachedXorshiro);
HWY_EXPORT_AND_TEST_P(PRISMXoshiroAPITest, TestUniformCachedXorshiro);

HWY_AFTER_TEST();
} // namespace
} // namespace hwy::HWY_NAMESPACE
HWY_TEST_MAIN();
#endif // HWY_ONCE