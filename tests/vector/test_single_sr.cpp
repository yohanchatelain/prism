#include <iostream>
#include <map>

// clang-format off
#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "tests/vector/test_single_sr.cpp"
#include "hwy/foreach_target.h"

#include "hwy/highway.h"
#include "hwy/base.h"
#include "hwy/tests/test_util-inl.h"
#include "src/debug_hwy-inl.h"
#include "src/sr_hw-inl.h"
// clang-format on

HWY_BEFORE_NAMESPACE(); // at file scope

namespace prism::sr::vector::HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

namespace {

struct TestOp {
  template <typename T, typename D>
  HWY_NOINLINE void operator()(T /*unused*/, D d) {

    const char *env_seed = getenv("PRISM_TEST_DEBUG");
    const bool debug = env_seed && std::string(env_seed) == "1";

    const std::size_t N = (debug) ? 1 : 10000;

    std::map<T, int> results;
    for (size_t i = 0; i < N; i++) {
      auto res = run<T>(d, debug);
      results[res[0]]++;
      results[res[1]]++;
    }

    if (not debug)
      if (results.size() != 2) {
        std::cerr << "Results size: " << results.size() << std::endl;
        std::cerr << "Results: ";
        for (const auto &r : results) {
          std::cerr << r.first << " ";
        }
        HWY_ASSERT_EQ(results.size(), 2);
      }
  }

  template <typename T> T get_user_input(int id) throw() {

    T value;
    bool initialized = false;

    const std::string env_name = "PRISM_TEST_SR_INPUT" + std::to_string(id);

    const char *env = getenv(env_name.c_str());

    if (env) {
      try {
        value = std::strtod(env, nullptr);
        initialized = true;
      } catch (const std::exception &e) {
      }
    }

    if (not initialized) {
      std::random_device rd;
      value = (double)rd();
    }

    return value;
  }

  std::string get_user_operation() throw() {
    static bool initialized = false;
    static std::string value = "add";

    if (initialized) {
      return value;
    }

    const char *env = getenv("PRISM_TEST_SR_OP");

    if (env) {
      value = env;
      initialized = true;
    }

    return value;
  }

  template <typename T, typename D>
  std::vector<T> run(D d, const bool debug = false) {
    constexpr T ulp = std::is_same_v<T, float> ? 0x1.0p-24 : 0x1.0p-53;
    constexpr const char *fmt = std::is_same_v<T, float> ? "%+.6a" : "%+.13a";

    const size_t elts = hn::Lanes(d);

    if (debug) {
      std::cout << "Type: " << typeid(T).name() << std::endl;
      std::cout << "Lanes: " << elts << std::endl;
    }

    auto op = get_user_operation();

    const T ai = (debug) ? get_user_input<T>(0) : 1.0;
    const T bi = (debug) ? get_user_input<T>(1) : ulp;
    T ci = 0;
    if (op == "fma") {
      ci = (debug) ? get_user_input<T>(2) : 1.0;
    }

    T aa[elts];
    T bb[elts];
    T cc[elts];

    for (size_t i = 0; i < elts; i++) {
      aa[i] = ai;
      bb[i] = bi;
      cc[i] = ci;
    }

    auto a = hn::Load(d, aa);
    auto b = hn::Load(d, bb);
    auto c = hn::Load(d, cc);

    if (debug) {
      hn::Print(d, "a", a, 0, 7, fmt);
      if (op != "sqrt")
        hn::Print(d, "b", b, 0, 7, fmt);
      if (op == "fma")
        hn::Print(d, "c", c, 0, 7, fmt);
    }

    hn::Vec<D> r;

    if (op == "add") {
      r = add<D>(a, b);
    } else if (op == "sub") {
      r = sub<D>(a, b);
    } else if (op == "mul") {
      r = mul<D>(a, b);
    } else if (op == "div") {
      r = div<D>(a, b);
    } else if (op == "sqrt") {
      r = sqrt<D>(a);
    } else if (op == "fma") {
      r = fma<D>(a, b, c);
    } else {
      std::cerr << "Unknown operation: " << op << std::endl;
      std::exit(1);
    }

    if (debug) {
      hn::Print(d, "r", r, 0, 7, fmt);
    }

    auto r_min = hn::ReduceMin(d, r);
    auto r_max = hn::ReduceMax(d, r);

    if (debug) {
      std::cout << std::hexfloat;
      std::cout << "Min: " << r_min << std::endl;
      std::cout << "Max: " << r_max << std::endl;
    }

    return {r_min, r_max};
  }
};

} // namespace

HWY_NOINLINE void TestAllOp() {
  hn::ForFloat3264Types(hn::ForPartialVectors<TestOp>());
}

// NOLINTNEXTLINE(google-readability-namespace-comments)
} // namespace prism::sr::vector::HWY_NAMESPACE
HWY_AFTER_NAMESPACE();

#if HWY_ONCE

namespace prism::sr::vector::HWY_NAMESPACE {
HWY_BEFORE_TEST(SRTest);
HWY_EXPORT_AND_TEST_P(SRTest, TestAllOp);
HWY_AFTER_TEST();
} // namespace prism::sr::vector::HWY_NAMESPACE

HWY_TEST_MAIN();

#endif // HWY_ONCE