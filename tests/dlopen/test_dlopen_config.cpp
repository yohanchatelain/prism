// Run-time options must reach libprism-static.so when an uninstrumented
// program loads an instrumented library with dlopen (verificarlo/prism#24).
//
// The program links no PRISM library: it dlopens, with RTLD_LOCAL like Python
// does, a backend that configures PRISM through libprism-dynamic.so and an
// instrumented library whose kernels live in libprism-static.so. Neither
// PRISM library is in the global scope, so the two only agree on the
// configuration if they share one copy of it. Before the fix each carried its
// own copy, and the static kernels kept stochastic rounding and their own seed
// whatever the backend set.
//
// Usage: test_dlopen_config <backend.so> <instrumented.so>

#include <dlfcn.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "src/prism_api.h"

namespace {

int failures = 0;

void check(bool ok, const char *what) {
  std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", what);
  if (not ok) {
    failures++;
  }
}

auto open_local(const char *path) -> void * {
  void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if (handle == nullptr) {
    std::fprintf(stderr, "dlopen %s: %s\n", path, dlerror());
    std::exit(2);
  }
  return handle;
}

template <typename F> auto lookup(void *handle, const char *name) -> F {
  void *symbol = dlsym(handle, name);
  if (symbol == nullptr) {
    std::fprintf(stderr, "dlsym %s: %s\n", name, dlerror());
    std::exit(2);
  }
  return reinterpret_cast<F>(symbol);
}

} // namespace

auto main(int argc, char *argv[]) -> int {
  if (argc != 3) {
    std::fprintf(stderr, "usage: %s <backend.so> <instrumented.so>\n",
                 argv[0]);
    return 2;
  }

  void *backend = open_local(argv[1]);
  void *instrumented = open_local(argv[2]);

  auto configure =
      lookup<void (*)(int32_t, uint64_t)>(backend, "prism_test_configure");
  auto sum = lookup<double (*)(int)>(instrumented, "prism_test_sum");
  auto rounding_mode =
      lookup<int32_t (*)()>(instrumented, "prism_test_rounding_mode");
  auto seed = lookup<uint64_t (*)()>(instrumented, "prism_test_seed");

  constexpr uint64_t kSeed = 0x5eed;
  configure(INTERFLOP_PRISM_RN, kSeed);

  check(rounding_mode() == INTERFLOP_PRISM_RN,
        "static kernels see the rounding mode set through the backend");
  check(seed() == kSeed, "static kernels see the seed set through the backend");

  // At full precision, PRISM's round-to-nearest agrees with IEEE addition on
  // this sum, which has no ties.
  double expected = 0;
  for (int i = 0; i < 1000; i++) {
    expected += 0.1;
  }
  bool deterministic = true;
  for (int run = 0; run < 3; run++) {
    const double got = sum(1000);
    std::printf("sum(1000) = %a (expected %a)\n", got, expected);
    deterministic = deterministic and got == expected;
  }
  check(deterministic, "static kernels round to nearest");

  return failures == 0 ? 0 : 1;
}
