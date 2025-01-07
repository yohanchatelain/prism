load("//tests:macros.bzl", "cc_test_gen_vector", "cc_test_lib_gen")

HWY_OPTS = [
    "-std=c++17",
    "-O2",
    "-Wfatal-errors",
]

# Tests
cc_test_gen_vector(
    name = "test_get_pow2",
    copts = HWY_OPTS,
    dbg = True,
)

cc_test_gen_vector(
    name = "test_get_exponent",
    copts = HWY_OPTS,
    dbg = True,
)

cc_test_lib_gen(
    name = "xoshiro",
    size = "medium",
    src = [
        "//src:srcs-xoshiro",
        "//tests/vector:test_xoshiro.cpp",
    ],
    copts = HWY_OPTS,
    dbg = True,
)

cc_test_lib_gen(
    name = "sr-accuracy",
    size = "large",
    src = [
        "//tests/vector:test_sr_accuracy.cpp",
    ],
    copts = HWY_OPTS,
    deps = [
        "@//src:prism-dynamic-dbg",
    ],
)

cc_test_lib_gen(
    name = "ud-accuracy",
    size = "medium",
    src = [
        "//tests/vector:test_ud_accuracy.cpp",
    ],
    copts = HWY_OPTS,
    deps = ["//src:prism-dynamic-dbg"],
)

cc_test_lib_gen(
    name = "sr-accuracy-dbg",
    size = "medium",
    src = [
        "//tests/vector:test_sr_accuracy.cpp",
        "//src:prism-dynamic-dbg",
    ],
    copts = HWY_OPTS,
    dbg = True,
)

cc_test_lib_gen(
    name = "thread",
    size = "medium",
    src = [
        "//tests/vector:test_threads.cpp",
    ],
    copts = HWY_OPTS + [
        "-fsanitize=address",
        "-DPRISM_DISPATCH=dynamic_dispatch",
    ],
    linkopts = [
        "-fsanitize=address",
    ],
    deps = [
        "@//src:prism-dynamic-dbg",
    ],
)

cc_test_lib_gen(
    name = "thread-static",
    size = "medium",
    src = [
        "//tests/vector:test_threads.cpp",
    ],
    copts = HWY_OPTS + [
        "-march=native",
        "-fsanitize=thread",
    ],
    linkopts = [
        "-fsanitize=thread",
    ],
    deps = [
        "@//src:prism-static-dbg",
    ],
)

# Performance tests
cc_test_lib_gen(
    name = "sr-perf",
    size = "medium",
    src = ["//tests/vector:test_sr_performance.cpp"],
    copts = HWY_OPTS + [
        "-Wno-psabi",
        "-fno-omit-frame-pointer",
        "-fsanitize=address",
    ],
    linkopts = [
        "-fsanitize=address",
    ],
    deps = [
        "@//src:prism-dynamic-dbg",
        "@hwy//:nanobenchmark",
    ],
)

cc_test_lib_gen(
    name = "sr-perf-static",
    size = "medium",
    src = ["//tests/vector:test_sr_performance.cpp"],
    copts = HWY_OPTS + [
        "-march=native",
        "-Wno-psabi",
    ],
    deps = [
        "@//src:prism-static",
        "@hwy//:nanobenchmark",
    ],
)

cc_test_lib_gen(
    name = "ud-perf",
    size = "medium",
    src = ["//tests/vector:test_ud_performance.cpp"],
    copts = HWY_OPTS + [
        "-Wno-psabi",
    ],
    deps = [
        "@//src:prism-dynamic",
        "@hwy//:nanobenchmark",
    ],
)

cc_test_lib_gen(
    name = "ud-perf-static",
    size = "medium",
    src = ["//tests/vector:test_ud_performance.cpp"],
    copts = HWY_OPTS + [
        "-march=native",
        "-Wno-psabi",
    ],
    deps = [
        "@//src:prism-native",
        "@hwy//:nanobenchmark",
    ],
)

# Single test for debugging purposes
# Not actual tests
cc_test_lib_gen(
    name = "sr-single-test",
    src = [
        "//tests/vector:test_single_sr.cpp",
    ],
    copts = HWY_OPTS,
    linkopts = [
        "-fsanitize=address",
    ],
    deps = [
        "@//src:prism-dynamic-dbg",
    ],
)

cc_test_lib_gen(
    name = "ud-single-test",
    src = [
        "//tests/vector:test_single_ud.cpp",
    ],
    copts = HWY_OPTS,
    linkopts = [
        "-fsanitize=address",
    ],
    deps = [
        "@//src:prism-dynamic-dbg",
    ],
)

test_suite(
    name = "all",
    tests = [
        ":sr-accuracy",
        ":test_get_pow2",
        ":thread",
        ":thread-static",
        ":ud-accuracy",
        ":xoshiro",
    ],
)

test_suite(
    name = "perf",
    tests = [
        ":sr-perf",
        ":sr-perf-native",
        ":ud-perf",
        ":ud-perf-native",
    ],
)
