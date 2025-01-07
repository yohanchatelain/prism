load("//tests:macros.bzl", "cc_test_gen_scalar", "cc_test_lib_gen")

HWY_OPTS = [
    "-std=c++17",
    "-O2",
    "-Wfatal-errors",
]

cc_test_gen_scalar(
    name = "test_get_exponent",
    copts = HWY_OPTS,
)

cc_test_gen_scalar(
    name = "test_get_predabs",
    copts = HWY_OPTS,
)

cc_test_gen_scalar(
    name = "test_get_pow2",
    copts = HWY_OPTS,
)

cc_test_gen_scalar(
    name = "test_twosum",
    copts = HWY_OPTS,
)

cc_test_gen_scalar(
    name = "test_twoprodfma",
    copts = HWY_OPTS,
)

cc_test_lib_gen(
    name = "sr-accuracy",
    size = "small",
    src = [
        "//tests/scalar:test_sr_accuracy.cpp",
    ],
    copts = HWY_OPTS,
    deps = ["//src:prism-dynamic"],
)

cc_test_lib_gen(
    name = "sr-accuracy-dbg",
    size = "small",
    src = [
        "//tests/scalar:test_sr_accuracy.cpp",
    ],
    copts = HWY_OPTS,
    dbg = True,
    deps = ["//src:prism-dynamic"],
)

cc_test_lib_gen(
    name = "ud-accuracy",
    size = "small",
    src = [
        "//tests/scalar:test_ud_accuracy.cpp",
    ],
    copts = HWY_OPTS,
    deps = ["//src:prism-dynamic"],
)

# Single tests for testing specific inputs
# These tests are useful for debugging
cc_binary(
    name = "sr-single-test",
    srcs = ["//tests/scalar:test_single_sr.cpp"],
    copts = HWY_OPTS + ["-DSR_DEBUG"],
    deps = ["//src:prism-dynamic"],
)

cc_binary(
    name = "ud-single-test",
    srcs = ["//tests/scalar:test_single_ud.cpp"],
    copts = HWY_OPTS + ["-DSR_DEBUG"],
    deps = ["//src:prism-dynamic"],
)

test_suite(
    name = "all",
    tests = [
        ":sr-accuracy",
        ":test_get_exponent",
        ":test_get_pow2",
        ":test_get_predabs",
        ":test_twoprodfma",
        ":test_twosum",
        ":ud-accuracy",
    ],
)
