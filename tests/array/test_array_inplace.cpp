#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_array.h"

namespace {

template <typename T>
void TestInPlaceOperations() {
  const size_t N = 127; // Odd size crossing multiple vector lanes

  // In-place add: a += b (result == a)
  {
    std::vector<T> a(N, static_cast<T>(2.0));
    std::vector<T> b(N, static_cast<T>(3.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), a.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::addf64(a.data(), b.data(), a.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(a[i], static_cast<T>(5.0), static_cast<T>(1e-4));
    }
  }

  // In-place add: b = a + b (result == b)
  {
    std::vector<T> a(N, static_cast<T>(2.0));
    std::vector<T> b(N, static_cast<T>(3.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), b.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::addf64(a.data(), b.data(), b.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(b[i], static_cast<T>(5.0), static_cast<T>(1e-4));
    }
  }

  // In-place sub: a -= b (result == a)
  {
    std::vector<T> a(N, static_cast<T>(10.0));
    std::vector<T> b(N, static_cast<T>(3.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::subf32(a.data(), b.data(), a.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::subf64(a.data(), b.data(), a.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(a[i], static_cast<T>(7.0), static_cast<T>(1e-4));
    }
  }

  // In-place mul: a *= b (result == a)
  {
    std::vector<T> a(N, static_cast<T>(4.0));
    std::vector<T> b(N, static_cast<T>(2.5));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::mulf32(a.data(), b.data(), a.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::mulf64(a.data(), b.data(), a.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(a[i], static_cast<T>(10.0), static_cast<T>(1e-4));
    }
  }

  // In-place div: a /= b (result == a)
  {
    std::vector<T> a(N, static_cast<T>(12.0));
    std::vector<T> b(N, static_cast<T>(3.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::divf32(a.data(), b.data(), a.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::divf64(a.data(), b.data(), a.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(a[i], static_cast<T>(4.0), static_cast<T>(1e-4));
    }
  }

  // In-place sqrt: a = sqrt(a)
  {
    std::vector<T> a(N, static_cast<T>(16.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::sqrtf32(a.data(), a.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::sqrtf64(a.data(), a.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(a[i], static_cast<T>(4.0), static_cast<T>(1e-4));
    }
  }

  // In-place FMA: c = fma(a, b, c) (result == c)
  {
    std::vector<T> a(N, static_cast<T>(2.0));
    std::vector<T> b(N, static_cast<T>(3.0));
    std::vector<T> c(N, static_cast<T>(4.0));
    if constexpr (std::is_same_v<T, float>) {
      prism::sr::array::PRISM_DISPATCH::fmaf32(a.data(), b.data(), c.data(), c.data(), N);
    } else {
      prism::sr::array::PRISM_DISPATCH::fmaf64(a.data(), b.data(), c.data(), c.data(), N);
    }
    for (size_t i = 0; i < N; ++i) {
      EXPECT_NEAR(c[i], static_cast<T>(10.0), static_cast<T>(1e-4));
    }
  }
}

} // namespace

TEST(ArrayInPlaceTest, SRFloatInPlace) {
  TestInPlaceOperations<float>();
}

TEST(ArrayInPlaceTest, SRDoubleInPlace) {
  TestInPlaceOperations<double>();
}
