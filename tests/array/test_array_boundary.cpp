#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "src/prism_array.h"
#include "src/sr_vector.h"
#include "src/ud_vector.h"

namespace {

const std::vector<size_t> test_counts = {
    0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 97, 1023, 1024, 1025
};

template <typename T>
void TestSRArrayOperations(size_t count) {
  std::vector<T> a(count + 4, static_cast<T>(1.5));
  std::vector<T> b(count + 4, static_cast<T>(2.25));
  std::vector<T> c(count + 4, static_cast<T>(0.5));
  std::vector<T> res(count + 4, static_cast<T>(-999.0));

  // Sentinel values after count to check no buffer overflow
  const T sentinel = static_cast<T>(-999.0);

  if constexpr (std::is_same_v<T, float>) {
    // Add
    prism::sr::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.75f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    // Sub
    prism::sr::array::PRISM_DISPATCH::subf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], -0.75f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    // Mul
    prism::sr::array::PRISM_DISPATCH::mulf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.375f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    // Div
    prism::sr::array::PRISM_DISPATCH::divf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 1.5f / 2.25f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    // Sqrt
    prism::sr::array::PRISM_DISPATCH::sqrtf32(a.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], std::sqrt(1.5f), 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    // FMA
    prism::sr::array::PRISM_DISPATCH::fmaf32(a.data(), b.data(), c.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.875f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }
  } else {
    // Double precision
    prism::sr::array::PRISM_DISPATCH::addf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.75, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::sr::array::PRISM_DISPATCH::subf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], -0.75, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::sr::array::PRISM_DISPATCH::mulf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.375, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::sr::array::PRISM_DISPATCH::divf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 1.5 / 2.25, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::sr::array::PRISM_DISPATCH::sqrtf64(a.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], std::sqrt(1.5), 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::sr::array::PRISM_DISPATCH::fmaf64(a.data(), b.data(), c.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.875, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }
  }
}

template <typename T>
void TestUDArrayOperations(size_t count) {
  std::vector<T> a(count + 4, static_cast<T>(1.5));
  std::vector<T> b(count + 4, static_cast<T>(2.25));
  std::vector<T> c(count + 4, static_cast<T>(0.5));
  std::vector<T> res(count + 4, static_cast<T>(-999.0));
  const T sentinel = static_cast<T>(-999.0);

  if constexpr (std::is_same_v<T, float>) {
    prism::ud::array::PRISM_DISPATCH::addf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.75f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::subf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], -0.75f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::mulf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.375f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::divf32(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 1.5f / 2.25f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::sqrtf32(a.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], std::sqrt(1.5f), 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::fmaf32(a.data(), b.data(), c.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.875f, 1e-5f);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }
  } else {
    prism::ud::array::PRISM_DISPATCH::addf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.75, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::subf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], -0.75, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::mulf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.375, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::divf64(a.data(), b.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 1.5 / 2.25, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::sqrtf64(a.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], std::sqrt(1.5), 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }

    prism::ud::array::PRISM_DISPATCH::fmaf64(a.data(), b.data(), c.data(), res.data(), count);
    for (size_t i = 0; i < count; ++i) {
      EXPECT_NEAR(res[i], 3.875, 1e-12);
    }
    for (size_t i = count; i < count + 4; ++i) {
      EXPECT_EQ(res[i], sentinel);
    }
  }
}

} // namespace

TEST(ArrayBoundaryTest, SRFloatCounts) {
  for (size_t count : test_counts) {
    TestSRArrayOperations<float>(count);
  }
}

TEST(ArrayBoundaryTest, SRDoubleCounts) {
  for (size_t count : test_counts) {
    TestSRArrayOperations<double>(count);
  }
}

TEST(ArrayBoundaryTest, UDFloatCounts) {
  for (size_t count : test_counts) {
    TestUDArrayOperations<float>(count);
  }
}

TEST(ArrayBoundaryTest, UDDoubleCounts) {
  for (size_t count : test_counts) {
    TestUDArrayOperations<double>(count);
  }
}
