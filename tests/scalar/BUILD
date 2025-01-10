load("//tests:macros.bzl", "cc_test_binary", "cc_test_gen_scalar", "cc_test_lib_gen")

cc_test_gen_scalar(
    name = "test_get_exponent",
)

cc_test_gen_scalar(
    name = "test_get_predabs",
)

cc_test_gen_scalar(
    name = "test_get_pow2",
)

cc_test_gen_scalar(
    name = "test_twosum",
)

cc_test_gen_scalar(
    name = "test_twoprodfma",
)

cc_test_lib_gen(
    name = "sr-accuracy",
    size = "small",
    src = [
        "//tests/scalar:test_sr_accuracy.cpp",
    ],
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "sr-accuracy-dbg",
    size = "small",
    src = [
        "//tests/scalar:test_sr_accuracy.cpp",
    ],
    dbg = True,
    mode = "dynamic",
)

cc_test_lib_gen(
    name = "ud-accuracy",
    size = "small",
    src = [
        "//tests/scalar:test_ud_accuracy.cpp",
    ],
    mode = "dynamic",
)

# Single tests for testing specific inputs
# These tests are useful for debugging
cc_test_binary(
    name = "sr-single-test",
    src = ["//tests/scalar:test_single_sr.cpp"],
    dbg = True,
    mode = "dynamic",
)

cc_test_binary(
    name = "ud-single-test",
    src = ["//tests/scalar:test_single_ud.cpp"],
    dbg = True,
    mode = "dynamic",
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
