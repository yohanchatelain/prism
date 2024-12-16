// Test that multithreading works.
#include <future>
#include <iostream>
#include <random>
#include <set>
#include <thread>
#include <vector>

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_threads.cpp"
#include "hwy/foreach_target.h" // Include last.

#include "hwy/highway.h"
#include "hwy/tests/test_util-inl.h"
#include "src/sr_vector-inl.h"

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

namespace {

std::pair<double, double> stats(const std::vector<double> &x) {
  if (x.empty()) {
    throw std::runtime_error(
        "Input vector is empty, cannot compute statistics.");
  }

  double n = 0.0;
  double S = 0.0;
  double m = 0.0;

  for (const auto &x_i : x) {
    n += 1.0;
    double m_prev = m;
    m += (x_i - m) / n;
    S += (x_i - m) * (x_i - m_prev);
  }

  return {m, std::sqrt(S / n)};
}

struct TestThread {

  template <typename T, typename D> static void run(D d, std::promise<T> &&p) {
    const size_t N = 1000;
    auto counter = hn::Set(d, (T)0);
    std::mt19937 gen(0);
    std::uniform_real_distribution<T> rd(0, 1);
    auto a = hn::Set(d, 0.1);
    auto b = hn::Set(d, 0.01);
    for (size_t i = 0; i < N; i++) {
      auto c = add<D>(a, b);
      counter = add<D>(counter, c);
    }
    p.set_value(hn::GetLane(counter));
  }

  int get_num_threads() {
    const char *env_threads = getenv("PRISM_TEST_THREADS");
    if (env_threads) {
      return std::stoi(env_threads);
    }
    return std::max(4U, std::thread::hardware_concurrency());
  }

  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {
    const std::size_t N = get_num_threads();
#ifdef SR_DEBUG
    std::cout << "Number of threads: " << N << std::endl;
#endif

    std::vector<std::thread> threads;
    std::vector<std::future<T>> futures;
    std::vector<std::promise<T>> promises(N);

    for (size_t i = 0; i < N; i++) {
      promises[i] = std::promise<T>();
      futures.push_back(promises[i].get_future());
      threads.push_back(
          std::thread(TestThread::run<T, D>, d, std::move(promises[i])));
    }

    std::vector<T> results;
    for (auto &t : threads) {
      t.join();
    }

    // build a set of unique results
    std::set<T> unique_results;
    std::vector<double> results_vector;
    for (size_t i = 0; i < N; i++) {
      const auto result = futures[i].get();
      unique_results.insert(result);
      results_vector.push_back(result);
    }

    const auto [mean, stddev] = stats(results_vector);

    std::cerr << typeid(T).name() << " mean: " << mean << " std: " << stddev
              << std::endl;

    HWY_ASSERT(unique_results.size() != 1);
  }
};

} // namespace

HWY_NOINLINE void TestThreads() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestThread>());
}

} // namespace prism::sr::vector::HWY_NAMESPACE

HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace prism::sr::vector::HWY_NAMESPACE {

HWY_BEFORE_TEST(SRThreadTest);

HWY_EXPORT_AND_TEST_P(SRThreadTest, TestThreads);

HWY_AFTER_TEST();
} // namespace prism::sr::vector::HWY_NAMESPACE

HWY_TEST_MAIN();

#endif // HWY_ONCE