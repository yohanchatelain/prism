load("//:constants.bzl", "DYNAMIC_COPTS", "SANITIZE_COPTS")
load("//tests:macros.bzl", "cc_test_gen_vector", "cc_test_lib_gen")

XOSHIRO_DEBUGS_COPTS = [
    "-DPRISM_DEBUG_XOSHIRO",
]

# Tests
cc_test_gen_vector(
    name = "test_get_pow2",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_get_exponent",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_twosum",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_fma",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_dekkerprod",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_split",
    dbg = True,
)

cc_test_gen_vector(
    name = "test_splitbit",
    dbg = True,
)

cc_test_lib_gen(
    name = "xoshiro",
    size = "medium",
    src = [
        "//src:srcs-xoshiro",
        "//tests/vector:test_xoshiro.cpp",
    ],
    copts = XOSHIRO_DEBUGS_COPTS,
    dbg = True,
)

cc_test_lib_gen(
    name = "xoshiro-api",
    size = "medium",
    src = [
        "//src:srcs-xoshiro",
        "//tests/vector:test_xoshiro_api.cpp",
    ],
    copts = XOSHIRO_DEBUGS_COPTS,
    dbg = True,
)

cc_test_lib_gen(
    name = "sr-accuracy",
    size = "large",
    src = [
        "//tests/vector:test_sr_accuracy.cpp",
    ],
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "ud-accuracy",
    size = "medium",
    src = [
        "//tests/vector:test_ud_accuracy.cpp",
    ],
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "ud-accuracy-fullbits",
    size = "medium",
    src = [
        "//tests/vector:test_ud_accuracy.cpp",
    ],
    copts = [
        "-DPRISM_FULL_BITS",
    ],
    mode = "",
    deps = ["//src:prism-dynamic-fullbits-dbg"],
)

cc_test_lib_gen(
    name = "ud-accuracy-fullbits-dbg",
    size = "medium",
    src = [
        "//tests/vector:test_ud_accuracy.cpp",
    ],
    copts = [
        "-DPRISM_FULL_BITS",
    ],
    dbg = True,
    mode = "",
    deps = ["//src:prism-dynamic-fullbits-dbg"],
)

cc_test_lib_gen(
    name = "sr-accuracy-dbg",
    size = "medium",
    src = [
        "//tests/vector:test_sr_accuracy.cpp",
    ],
    dbg = True,
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "ud-accuracy-dbg",
    size = "medium",
    src = [
        "//tests/vector:test_ud_accuracy.cpp",
    ],
    dbg = True,
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "thread",
    size = "medium",
    src = [
        "//tests/vector:test_threads.cpp",
    ],
    copts = DYNAMIC_COPTS + SANITIZE_COPTS,
    linkopts = SANITIZE_COPTS,
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "thread-static",
    size = "medium",
    src = [
        "//tests/vector:test_threads.cpp",
    ],
    copts = [
        "-march=native",
        "-fsanitize=thread",
    ],
    linkopts = [
        "-fsanitize=thread",
    ],
    mode = "static",
    deps = [
        "@//src:prism-static-dbg",
    ],
)

# Performance tests
cc_test_lib_gen(
    name = "sr-perf",
    size = "medium",
    src = ["//tests/vector:test_sr_performance_dynamic.cpp"],
    copts = [
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
    src = ["//tests/vector:test_sr_performance_static.cpp"],
    copts = [
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
    src = ["//tests/vector:test_ud_performance_dynamic.cpp"],
    copts = [
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
    src = ["//tests/vector:test_ud_performance_static.cpp"],
    copts = [
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
    dbg = True,
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
    dbg = True,
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
